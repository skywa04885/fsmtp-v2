#include "UIDHolder.src.h"

using namespace FSMTP::Models;

void UIDHolder::getPrefix(
    const int64_t bucket, const string &domain,
    const CassUuid &uuid, char *buffer
) {
    char uuidBuffer[CASS_UUID_STRING_LENGTH];
    cass_uuid_string(uuid, uuidBuffer);

    sprintf(buffer, "UIDH:%ld:%s:%s", bucket, domain.c_str(), uuidBuffer);
}

int32_t UIDHolder::getAndIncrement(
    CassandraConnection *cass, RedisConnection *redis,
    const int64_t bucket, const string &domain,
    const CassUuid &uuid
) {
    int32_t largestUID;

    try {
        largestUID = UIDHolder::getRedis(redis, bucket, domain, uuid);
    } catch (const EmptyQuery &e) {
        largestUID = UIDHolder::restoreFromCassandra(cass, redis, bucket, domain, uuid);
    }

    largestUID++;
    UIDHolder::saveRedis(redis, bucket, domain, uuid, largestUID);;

    return largestUID;
}

void UIDHolder::saveRedis(
    RedisConnection *redis, const int64_t bucket,
    const string &domain, const CassUuid &uuid,
    const int32_t num
) {
    char prefix[512], command[768];

    UIDHolder::getPrefix(bucket, domain, uuid, prefix);
    sprintf(command, "SET %s %d", prefix, num);

    redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
        redis->r_Session, command
    ));
    DEFER(freeReplyObject(reply));

    if (reply->type == REDIS_REPLY_ERROR) {
        string error = "redisCommand() failed: ";
        error += string(reply->str, reply->len);
        throw DatabaseException(EXCEPT_DEBUG(error));
    }
}

int32_t UIDHolder::getRedis(
    RedisConnection *redis, const int64_t bucket,
    const string &domain, const CassUuid &uuid
) {
    char prefix[512], command[768];

    UIDHolder::getPrefix(bucket, domain, uuid, prefix);
    sprintf(command, "GET %s", prefix);

    redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
        redis->r_Session, command
    ));
    DEFER(freeReplyObject(reply));

    if (reply->type == REDIS_REPLY_ERROR) {
        string error = "redisCommand() failed: ";
        error += string(reply->str, reply->len);
        throw DatabaseException(EXCEPT_DEBUG(error));
    } else if (reply->type == REDIS_REPLY_NIL) {
        throw EmptyQuery("UID Could not be found");
    } return stoi(reply->str);
}

int32_t UIDHolder::restoreFromCassandra(
    CassandraConnection *cass, RedisConnection *redis,
    const int64_t bucket, const string &domain,
    const CassUuid &uuid
) {
    const char *query = R"(SELECT e_uid FROM fannst.email_shortcuts
    WHERE e_domain=? AND e_owners_uuid=? ALLOW FILTERING)";

    CassStatement *statement = cass_statement_new(query, 2);
    DEFER(cass_statement_free(statement));
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_uuid(statement, 1, uuid);
    cass_statement_set_paging_size(statement, 50);

    int32_t largestUID = 0;
    cass_bool_t hasMorePages;
    do {
        CassFuture *future = cass_session_execute(cass->c_Session, statement);
        DEFER(cass_future_free(future));
        cass_future_wait(future);

        if (cass_future_error_code(future) != CASS_OK) {
            string error = "cass_session_execute() failed: ";
            error += CassandraConnection::getError(future);
            throw DatabaseException(EXCEPT_DEBUG(error));
        }
        
        const CassResult *result = cass_future_get_result(future);
        CassIterator *iterator = cass_iterator_from_result(result);
        DEFER({
            cass_result_free(result);
            cass_iterator_free(iterator);
        });

        while (cass_iterator_next(iterator)) {
            const CassRow *row = cass_iterator_get_row(iterator);
            int32_t uid;

            cass_value_get_int32(cass_row_get_column_by_name(row, "e_uid"), &uid);

            if (uid > largestUID) largestUID = uid;
        }

        hasMorePages = cass_result_has_more_pages(result);
        if (hasMorePages) cass_statement_set_paging_state(statement, result);
    } while (hasMorePages);

    return largestUID;
}
