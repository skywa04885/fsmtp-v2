#include "../lib/dkim/DKIMVerifier.src.h"

using namespace FSMTP::DKIM;

// =============================
// // Default dkim tests
// =============================

TEST_CASE("Parse DKIM Header into KV pairs") {
	string raw = R"(DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=fannst.nl; s=default;
       h=subject:date:mime-version:message-id:to:from;
       bh=1r6joMzD65Y2I+kDL40gRc/oD2qi9fzYx4IcpkYniQw=; b=GMkrL9n0FVr87NRTSTksp3qs/FO6gU4UTLp7QP1
        Qxx3rGJD41aVpEFq7ZKHCcqO2b5k3QjbwaOwUeZfL8p5sksqnE2TXLbUA5iMfUr8vH/AtMw5E+7uH9Cn5Ye8JbZWXa
        f1jL2iC5BnrU4DSUtl2+XbhRBfsWkhiOA9pgD35azye/jUoXetG8ulw/bdUb8DWulUgGd9oRZybFpe2s54yao9dG0q
        /wSgn5h5GdSPE78uLwGGj/q14x9j1xHDW7EdXUHtkKWN9DtrRhI3L6da/TnYlW16AeR4nK+LQOaxaG9+qAKylJkaYK
        +70O+DuNwEumJP1Yhbyh/TS2+cf1zRdGw==)";
}
