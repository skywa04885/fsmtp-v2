![Fannst Banner](static/nogmail.png)

# Fannst ESMTP Server Versie 2.0

Een volledige SMTP / ESMTP Server, die binnen een korte
periode ook ondersteuning heeft voor relaying en SMTP Authentication.
Naast dit zal er ook een API bij komen, waarmee emails makkelijk verstuurd
kunnen worden uit andere applicaties.

## Wat kan FSMTP nu ?

1. Emails ontvangen met zowel een plain-text verbinding en beveiligde verbinding.
1. Ontvangen emails verwerken en opslaan in een database, gelinkt aan een gebruiker.
1. Emails weigeren wanneer de gebruiker niet in de database staat of wij geen eigenaar van het domein zijn.
1. Emails versturen met STARTTLS en andere dingen

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

## Welke systemen werkt FSMTP-V2 mee ?

FSMTP-V2 is getest en werkt op deze diensten:

1. Outlook en alle bedrijven die dit gebruiken
1. Yahoo Mail
1. Google Mail
1. Andere ESMTP & SMTP Servers

Een voorbeeld hiervan is:
```bash
T139638755706688->[debug@DNS Resolver]: Vinden van records voor: student.roc-nijmegen.nl
T139638755706688->[debug@DNS Resolver]: Antwoord lengte: 110/2048 octets
T139638755706688->[debug@DNS Resolver]: DNS Record aantal: 1
T139638755706688->[debug@DNS Resolver]: DNS Record [nm: 'student.roc-nijmegen.nl', type: 15, ttl: 300, class: 1, len: 57]: student-rocnijmegen-nl02i.mail.protection.outlook.com
T139638755706688->[informatie@SMTPClient]: Server toegevoegd: 104.47.2.36
T139638755706688->[informatie@SMTPClient]: Versturen naar 1 clients ..
T139638755706688->[informatie@SMTPClient]: Connected to: 104.47.2.36:25
T139638755706688->[debug@SMTPClient]: S->[code:220]: DB5EUR01FT013.mail.protection.outlook.com Microsoft ESMTP MAIL Service ready at Mon, 13 Jul 2020 11:41:54 +0000
T139638755706688->[debug@SMTPClient]: C->RESP: EHLO [domain]
T139638755706688->[debug@SMTPClient]: S->[code:250]: DB5EUR01FT013.mail.protection.outlook.com Hello [86.89.30.158]
T139638755706688->[debug@SMTPClient]: S->[code:250]: SIZE 157286400
T139638755706688->[debug@SMTPClient]: S->[code:250]: PIPELINING
T139638755706688->[debug@SMTPClient]: S->[code:250]: DSN
T139638755706688->[debug@SMTPClient]: S->[code:250]: ENHANCEDSTATUSCODES
T139638755706688->[debug@SMTPClient]: S->[code:250]: STARTTLS
T139638755706688->[debug@SMTPClient]: S->[code:250]: 8BITMIME
T139638755706688->[debug@SMTPClient]: S->[code:250]: BINARYMIME
T139638755706688->[debug@SMTPClient]: S->[code:250]: CHUNKING
T139638755706688->[debug@SMTPClient]: S->[code:250]: SMTPUTF8
T139638755706688->[debug@SMTPClient]: C->RESP: STARTTLS
T139638755706688->[debug@SMTPClient]: S->[code:220]: 2.0.0 SMTP server ready
T139638755706688->[debug@SMTPClient]: Omzetten naar TLS verbinding ...
T139638755706688->[debug@SMTPClient]: Verbinding omgezet !
T139638755706688->[debug@SMTPClient]: C->RESP: EHLO [domain]
T139638755706688->[debug@SMTPClient]: S->[code:250]: DB5EUR01FT013.mail.protection.outlook.com Hello [86.89.30.158]
T139638755706688->[debug@SMTPClient]: S->[code:250]: SIZE 157286400
T139638755706688->[debug@SMTPClient]: S->[code:250]: PIPELINING
T139638755706688->[debug@SMTPClient]: S->[code:250]: DSN
T139638755706688->[debug@SMTPClient]: S->[code:250]: ENHANCEDSTATUSCODES
T139638755706688->[debug@SMTPClient]: S->[code:250]: 8BITMIME
T139638755706688->[debug@SMTPClient]: S->[code:250]: BINARYMIME
T139638755706688->[debug@SMTPClient]: S->[code:250]: CHUNKING
T139638755706688->[debug@SMTPClient]: S->[code:250]: SMTPUTF8
T139638755706688->[debug@SMTPClient]: C->RESP: MAIL FROM: [sender]
T139638755706688->[debug@SMTPClient]: S->[code:250]: 2.1.0 Sender OK
T139638755706688->[debug@SMTPClient]: C->RESP: RCPT TO: [target]
T139638755706688->[debug@SMTPClient]: S->[code:250]: 2.1.5 Recipient OK
T139638755706688->[debug@SMTPClient]: S->[code:354]: Start mail input; end with <CRLF>.<CRLF>
T139638755706688->[debug@SMTPClient]: S->[code:250]: 2.6.0 <l4TusLpI7eQJzrsrK4icsZqB9fjvBWG1PVCuXfwq021KhSJmMzkdXeD2FDvI9Wg1@mail.fannst.nl> [InternalId=23592255360123, Hostname=VI1P194MB0110.EURP194.PROD.OUTLOOK.COM] 9204 bytes in 0.173, 51.834 KB/sec Queued mail for delivery
T139638755706688->[debug@SMTPClient]: S->[code:221]: 2.0.0 Service closing transmission channel
T139638755706688->[debug@SMTPClient]: Closing transmission channel ...
```

## Auteurs
|Name|Github|
|-|-|
|Luke Rieff|[Click here](https://github.com/skywa04885)|