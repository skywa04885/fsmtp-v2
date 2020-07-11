# Fannst ESMTP Server Versie 2.0

Een volledige SMTP / ESMTP Server, die binnen een korte
periode ook ondersteuning heeft voor relaying en SMTP Authentication.
Naast dit zal er ook een API bij komen, waarmee emails makkelijk verstuurd
kunnen worden uit andere applicaties.

## Wat kan FSMTP nu ?

1. Emails ontvangen met zowel een plain-text verbinding en beveiligde verbinding.
1. Ontvangen emails verwerken en opslaan in een database, gelinkt aan een gebruiker.
1. Emails weigeren wanneer de gebruiker niet in de database staat of wij geen eigenaar van het domein zijn.

## Wat bibliotheken heb ik nodig ?

1. HiRedis [Github](https://github.com/redis/hiredis)
1. DataStax CQL Client [Github](https://github.com/datastax/cpp-driver)

Van beide is de installatie best makkelijk:
```bash
mkdir build
cd ./build
cmake ..
make -j [CORES]
sudo make install
```

## Hoe installeer ik het ?

```bash
mkdir build
cmake ..
make -j [CORES]
sudo ./fsmtp
```

## Hoe gebruik ik het ?

```bash
sudo ./fsmtp
```

Bij het gebruiken van de FSMTP server kunt u ook opdrachten meegeven, als u ```-h``` als argument toevoegd, kunt u het help menu zien, deze zal ik hieronder voor het gemak zetten:

```bash
Gebruik: 
sudo fsmtp [arguments]

Opdrachten: 
-h, --help: 	Print de lijst met beschikbare opdrachten.
-t, --test: 	Voer tests uit op de vitale functies van de server, zoals database verbinding.
-s, --sync: 	Synchroniseerd de redis database met die van cassandra
-n, --ncurses: 	Gebruik NCurses in plaats van klassieke terminal.
```

## Auteurs
|Name|Github|
|-|-|
|Luke Rieff|[Click here](https://github.com/skywa04885)|