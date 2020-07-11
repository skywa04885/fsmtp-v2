# Fannst ESMTP Server Versie 2.0

Aangezien de vorige versie vol zat met fouten en geheugen lekkages, heb
ik besloten om een nieuwe te maken, die perfect zal functioneren. En ja,
ik gebruik de Nederlandse taal, ik ben er namelijk helemaal klaar mee dat
alles in ons land enkel wordt gedaan voor de internationale doelgroep, ik
houdt van Nederland, en zal in dit geval dat laten zien door de software in
het Nederlands te schijven, zover het uitkomt.

## Wat heb ik nodig ?

1. HiRedis [Github](https://github.com/redis/hiredis)
1. DataStax CQL Client [Github](https://github.com/datastax/cpp-driver)

## Hoe installeer ik het ?

```bash
mkdir build
cmake ..
make -j [CORES]
sudo ./fsmtp
```

## Auteurs
|Name|Github|
|-|-|
|Luke Rieff|[Click here](https://github.com/skywa04885)|