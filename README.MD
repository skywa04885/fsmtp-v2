![Fannst Banner](static/fannst-banner.png)

# Fannst ESMTP Server Versie 2.0

Een volledige SMTP / ESMTP Server, die binnen een korte
periode ook ondersteuning heeft voor relaying en SMTP Authentication.
Naast dit zal er ook een API bij komen, waarmee emails makkelijk verstuurd
kunnen worden uit andere applicaties.

## Wat kan FSMTP nu ?

1. Emails ontvangen, versturen met een STARTTLS verbinding
1. Emails opslaan met RSA beveiliging per gebruiker
1. Emails samen stellen, en versturen
1. Emails parsen
1. Communiceren met niet bekende ESMTP / SMTP Servers
1. Communiceren met bijvoorbeeld Thunderbird en andere email clients
1. Emails relayen vanaf een bestaand account, met authenticatie
1. Emails versturen met een CLI

## Waar wordt aan gewerkt ?

1. IMAP Server (IMAP4rev1)
1. Uitbereiding van de ESMTP server
1. Verbetering leesbaarheid code
1. DNS Server, die ook DKIM sleutels dagelijks update

## Wat bibliotheken heb ik nodig ?

1. JSON [Github](https://github.com/nlohmann/json)
1. Hayai [Github](https://github.com/nickbruun/hayai)
1. HiRedis [Github](https://github.com/redis/hiredis)
1. DataStax CQL Client [Github](https://github.com/datastax/cpp-driver)
1. Inja Template Engine [Github](https://github.com/pantor/inja)
1. JSONCPP [Github](https://github.com/open-source-parsers/jsoncpp)
1. Catch2 [Github](https://github.com/catchorg/Catch2)

## Hoe installeer ik het ?

```bash
sudo apt-get install meson ninja
chmod +x ./install.sh
./install.sh
meson build && cd ./build
ninja
./fsmtp
```

## Hoe gebruik ik het ?

1. Maak een map genaamd env/keys
1. Doe uw SSL certificaat erin met de naam 'cert.pem'
1. Doe uw SSL sleutel erin met de naam 'key.pem'
1. Plaats uw DKIM RSA-Keypair erin met de namen: 'dkim-private.pem', 'dkim-public.pem'
1. Vergeet niet de publieke sleuten van uw DKIM keypair the plaatsen in uw DNS server onder de naam 'default.\_keyselector'

```bash
sudo ./fsmtp -n
```

Bij het gebruiken van de FSMTP server kunt u ook opdrachten meegeven, als u ```-h``` als argument toevoegd, kunt u het help menu zien, deze zal ik hieronder voor het gemak zetten:

```bash
Gebruik: 
sudo fsmtp [arguments]

Opdrachten: 
-h, -help: 	Print de lijst met beschikbare opdrachten.
-t, -test: 	Voer tests uit op de vitale functies van de server, zoals database verbinding.
-s, -sync: 	Synchroniseerd de redis database met die van cassandra
-a, -adduser:	Adds an new user to the email server.
-m, -mailtest: 	Sends an email.
-r, -run=type: 	Welke server er gestart moet worden, 'smtp' of 'pop3'
```

## Welke systemen werkt FSMTP-V2 mee ?

FSMTP-V2 is getest en werkt op deze diensten:

1. Outlook en alle bedrijven die dit gebruiken
1. Yahoo Mail
1. Google Mail
1. Andere ESMTP & SMTP Servers


een voorbeeld van een ESMTP Conversatie ( intern, met FSMTP-V2 Extensions ):

```
S -> 220 LUKERIEFF_MCLUST_A001 Fannst ESMTP Mail service ready at Fri, 11 Sep 2020 20:07:18 CEST - fsmtp
C -> FCAPA
S -> 601 6.1.1 FSMTP-V2 Extensions implemented, Proceed. - fsmtp
C -> SU
S -> 600 6.1.0 Access granted for 145-53-228-5.fixed.kpn.net [145.53.228.5] - fsmtp
C -> EHLO fannst.nl
S -> 250-smtp.fannst.nl, at your service 145-53-228-5.fixed.kpn.net [145.53.228.5]
   + 250-AUTH PLAIN
   + 250-STARTTLS
   + 250-SMTPUTF8
   + 250-SIZE 10485760
   + 250 ENHANCEDSTATUSCODES
C -> STARTTLS
S -> 220 5.7.0 Ready to start TLS - fsmtp

(( Upgrading to TLS ))

C -> FCAPA
S -> 601 6.1.1 FSMTP-V2 Extensions implemented, Proceed. - fsmtp
C -> SU
S -> 600 6.1.0 Access granted for 145-53-228-5.fixed.kpn.net [145.53.228.5] - fsmtp
C -> EHLO fannst.nl
S -> 250-smtp.fannst.nl, at your service 145-53-228-5.fixed.kpn.net [145.53.228.5]
   + 250-AUTH PLAIN
   + 250-SMTPUTF8
   + 250-SIZE 10485760
   + 250 ENHANCEDSTATUSCODES
C -> MAIL FROM: lrieff@fannst.nl
S -> 250 2.1.0 OK, proceed [luke.rieff@gmail.com] - fsmtp
C -> RCPT TO: webmaster@fannst.nl
S -> 250 2.1.5 OK, proceed [webmaster@fannst.nl] - fsmtp
C -> DATA
S -> 354 End data with <CR><LF>.<CR><LF> - fsmtp

(( Sends message body ))

S -> 250 2.6.0 OK, message queued <P2luzwlzea7BvtPdW4hM6k5ymOQqX7kZuP9O2M8qXXa0S5qt4M7owG13PzaqTYPV@mailer.fannst.nl> - fsmtp
C -> QUIT
S -> 221 2.0.0 Closing connection - fsmtp
```

Een voorbeeld van de Fannst Mailer:

```bash
Fri, 11 Sep 2020 20:00:15->(INFO@Mail): Enter type [c: custom, d: default]: c
Fri, 11 Sep 2020 20:00:16->(INFO@Mail): Enter subject: Test Message
Fri, 11 Sep 2020 20:00:19->(INFO@Mail): Enter from: webmaster@fannst.nl
Fri, 11 Sep 2020 20:00:23->(INFO@Mail): Enter to: luke.rieff@gmail.com
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Voorbereiden ...
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Timer@MessageComposer started !
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Begonnen met opbouw van MIME Message
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Verwerkings datum: Fri, 11 Sep 2020 20:00:26 (CEST)
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Bericht ID: <lCQVaz0IBgu10EtILY4lg9p9vTNYxYOpFQqvsfAGdPnzWPgBZKmsXNryZYrPvfTc@mailer.fannst.nl>
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Van: <webmaster@fannst.nl>
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Naar: <luke.rieff@gmail.com>
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): sections.size() == 0, generating default one ...
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Result: '
X-Mailer: FSMTP-V2
X-Fannst-NodeID: LUKERIEFF_MCLUST_A001
X-Made-By: Luke A.C.A. Rieff
Subject: Test Message
Date: Fri, 11 Sep 2020 20:00:26 (CEST)
MIME-Version: 1.0
Message-ID: <lCQVaz0IBgu10EtILY4lg9p9vTNYxYOpFQqvsfAGdPnzWPgBZKmsXNryZYrPvfTc@mailer.fannst.nl>
To: <luke.rieff@gmail.com>
From: <webmaster@fannst.nl>
Content-Type: multipart/alternative; boundary="--tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL"

----tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL
Content-Type: text/html; charset="utf-8"
Content-Transfer-Encoding: quoted-printable

<p>
<strong>Roses are red.</strong><br />
<small>Violets are blue.</small><br />
<em>Someone did NOT specify an message body.</em></p>

----tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL--
'
Fri, 11 Sep 2020 20:00:26->(DEBUG@MailComposer): Timer@MessageComposer ended in [0.555776ms, 555959us]
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Timer@Signer started !
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Microsoft rotzooi header genegeerd: X-Mailer
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Microsoft rotzooi header genegeerd: X-Fannst-NodeID
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Microsoft rotzooi header genegeerd: X-Made-By
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:0]: 'subject' -> 'Test Message'
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:1]: 'date' -> 'Fri, 11 Sep 2020 20:00:26 (CEST)'
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:2]: 'mime-version' -> '1.0'
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:3]: 'message-id' -> '<lCQVaz0IBgu10EtILY4lg9p9vTNYxYOpFQqvsfAGdPnzWPgBZKmsXNryZYrPvfTc@mailer.fannst.nl>'
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:4]: 'to' -> '<luke.rieff@gmail.com>'
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:5]: 'from' -> '<webmaster@fannst.nl>'
Fri, 11 Sep 2020 20:00:26->(PARSER@HeaderParser): Header [no:6]: 'content-type' -> 'multipart/alternative; boundary="--tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL"'
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Started canonicalization of headers and body
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Canonicalized headers: 
'subject:Test Message
date:Fri, 11 Sep 2020 20:00:26 (CEST)
mime-version:1.0
message-id:<lCQVaz0IBgu10EtILY4lg9p9vTNYxYOpFQqvsfAGdPnzWPgBZKmsXNryZYrPvfTc@mailer.fannst.nl>
to:<luke.rieff@gmail.com>
from:<webmaster@fannst.nl>'
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Canonicalized body: 
'----tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL
Content-Type: text/html; charset="utf-8"
Content-Transfer-Encoding: quoted-printable

<p>
<strong>Roses are red.</strong><br />
<small>Violets are blue.</small><br />
<em>Someone did NOT specify an message body.</em></p>

----tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL--'
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Generated body hash: f+DQL8U72ZwLu1NyiQFXqMS1AQgzoJDNJ6IYIVta2xU=
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Raw signature: 
'subject:Test Message
date:Fri, 11 Sep 2020 20:00:26 (CEST)
mime-version:1.0
message-id:<lCQVaz0IBgu10EtILY4lg9p9vTNYxYOpFQqvsfAGdPnzWPgBZKmsXNryZYrPvfTc@mailer.fannst.nl>
to:<luke.rieff@gmail.com>
from:<webmaster@fannst.nl>
dkim-signature:v=1; a=rsa-sha256; c=relaxed/relaxed; d=fannst.nl; s=default; h=subject:date:mime-version:message-id:to:from; bh=f+DQL8U72ZwLu1NyiQFXqMS1AQgzoJDNJ6IYIVta2xU=; b='
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Generated signature: ZVIvb3itCw1U2N8u7ktjkzKDZYqsVNK9nuKjPicIIsoXpDR2zI+Rq+jSzGOWVIdnbBgXiFAlyz9kFN9Bv1Wxd3Hth+LAEWyakxw4NUTMOLHNoMWEmi4RXg2NdSf7q4wJUYid3elnqGlwhCMF2Mc3ZpL5nAk/905X1AdB2nse3mns6WVJq/yG+GCyvBHhWcV/jHytFugW6K+3Bwz+PhBD4TA9phsyTae2lPDqLOEfK7oRX5s616RsQnnu3Bom95gjXHH88zM6lYp48kXw64phRAbWRoQWfq7PVjqckgOEfmtVbSJ6idKBUQe2NoYVFHIul3xVyiaT2JQfiPMPi6ydZQ==
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Final DKIM Signature: 
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=fannst.nl; s=default; 
       h=subject:date:mime-version:message-id:to:from; 
       bh=f+DQL8U72ZwLu1NyiQFXqMS1AQgzoJDNJ6IYIVta2xU=; b=ZVIvb3itCw1U2N8u7ktjkzKDZYqsVNK9nuKjPic
        IIsoXpDR2zI+Rq+jSzGOWVIdnbBgXiFAlyz9kFN9Bv1Wxd3Hth+LAEWyakxw4NUTMOLHNoMWEmi4RXg2NdSf7q4wJU
        Yid3elnqGlwhCMF2Mc3ZpL5nAk/905X1AdB2nse3mns6WVJq/yG+GCyvBHhWcV/jHytFugW6K+3Bwz+PhBD4TA9phs
        yTae2lPDqLOEfK7oRX5s616RsQnnu3Bom95gjXHH88zM6lYp48kXw64phRAbWRoQWfq7PVjqckgOEfmtVbSJ6idKBU
        Qe2NoYVFHIul3xVyiaT2JQfiPMPi6ydZQ==
Fri, 11 Sep 2020 20:00:26->(DEBUG@DKIMSigner): Timer@Signer ended in [3.5735ms, 3573530us]
X-Mailer: FSMTP-V2
X-Fannst-NodeID: LUKERIEFF_MCLUST_A001
X-Made-By: Luke A.C.A. Rieff
Subject: Test Message
Date: Fri, 11 Sep 2020 20:00:26 (CEST)
MIME-Version: 1.0
Message-ID: <lCQVaz0IBgu10EtILY4lg9p9vTNYxYOpFQqvsfAGdPnzWPgBZKmsXNryZYrPvfTc@mailer.fannst.nl>
To: <luke.rieff@gmail.com>
From: <webmaster@fannst.nl>
Content-Type: multipart/alternative; boundary="--tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL"
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=fannst.nl; s=default; 
       h=subject:date:mime-version:message-id:to:from; 
       bh=f+DQL8U72ZwLu1NyiQFXqMS1AQgzoJDNJ6IYIVta2xU=; b=ZVIvb3itCw1U2N8u7ktjkzKDZYqsVNK9nuKjPic
        IIsoXpDR2zI+Rq+jSzGOWVIdnbBgXiFAlyz9kFN9Bv1Wxd3Hth+LAEWyakxw4NUTMOLHNoMWEmi4RXg2NdSf7q4wJU
        Yid3elnqGlwhCMF2Mc3ZpL5nAk/905X1AdB2nse3mns6WVJq/yG+GCyvBHhWcV/jHytFugW6K+3Bwz+PhBD4TA9phs
        yTae2lPDqLOEfK7oRX5s616RsQnnu3Bom95gjXHH88zM6lYp48kXw64phRAbWRoQWfq7PVjqckgOEfmtVbSJ6idKBU
        Qe2NoYVFHIul3xVyiaT2JQfiPMPi6ydZQ==

----tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL
Content-Type: text/html; charset="utf-8"
Content-Transfer-Encoding: quoted-printable

<p>
<strong>Roses are red.</strong><br />
<small>Violets are blue.</small><br />
<em>Someone did NOT specify an message body.</em></p>

----tYnHu7fRxVsm3dcHYEa7KjZuk9hy8bVkfTDiWL--

Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): Vinden van records voor: gmail.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): Antwoord lengte: 150/2048 octets
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): DNS Record aantal: 5
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): DNS Record [nm: 'gmail.com', type: 15, ttl: 3430, class: 1, len: 27]: gmail-smtp-in.l.google.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): DNS Record [nm: 'gmail.com', type: 15, ttl: 3430, class: 1, len: 9]: alt4.gmail-smtp-in.l.google.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): DNS Record [nm: 'gmail.com', type: 15, ttl: 3430, class: 1, len: 9]: alt3.gmail-smtp-in.l.google.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): DNS Record [nm: 'gmail.com', type: 15, ttl: 3430, class: 1, len: 9]: alt2.gmail-smtp-in.l.google.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@DNS Resolver): DNS Record [nm: 'gmail.com', type: 15, ttl: 3430, class: 1, len: 9]: alt1.gmail-smtp-in.l.google.com
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Server toegevoegd: 108.177.127.27
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Server toegevoegd: 173.194.201.26
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Server toegevoegd: 74.125.28.26
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Server toegevoegd: 108.177.97.27
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Server toegevoegd: 142.250.4.27
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Versturen naar 1 clients ..
Fri, 11 Sep 2020 20:00:26->(INFO@SMTPClient): Connected to: 108.177.127.27:25
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:220]: mx.google.com ESMTP u2si1917672edx.5 - gsmtp
gmail.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): C->RESP: EHLO [domain]
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: mx.google.com at your service, [145.53.228.5]
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: SIZE 157286400
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: 8BITMIME
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: STARTTLS
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: ENHANCEDSTATUSCODES
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: PIPELINING
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: CHUNKING
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: SMTPUTF8
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): C->RESP: STARTTLS
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:220]: 2.0.0 Ready to start TLS
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): Omzetten naar TLS verbinding ...
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): Verbinding omgezet !
gmail.com
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): C->RESP: EHLO [domain]
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: mx.google.com at your service, [145.53.228.5]
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: SIZE 157286400
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: 8BITMIME
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: ENHANCEDSTATUSCODES
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: PIPELINING
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: CHUNKING
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: SMTPUTF8
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): C->RESP: MAIL FROM: [sender]
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: 2.1.0 OK u2si1917672edx.5 - gsmtp
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): C->RESP: RCPT TO: [target]
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:250]: 2.1.5 OK u2si1917672edx.5 - gsmtp
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): S->[code:354]: Go ahead u2si1917672edx.5 - gsmtp
Fri, 11 Sep 2020 20:00:26->(DEBUG@SMTPClient): C->RESP: DATA [BODY]
Fri, 11 Sep 2020 20:00:27->(DEBUG@SMTPClient): S->[code:250]: 2.0.0 OK 1599847265 u2si1917672edx.5 - gsmtp
Fri, 11 Sep 2020 20:00:27->(DEBUG@SMTPClient): S->[code:221]: 2.0.0 closing connection u2si1917672edx.5 - gsmtp
Fri, 11 Sep 2020 20:00:27->(DEBUG@SMTPClient): Closing transmission channel ...
Fri, 11 Sep 2020 20:00:27->(INFO@SMTPClient): Handled 1 emails
```
Een voorbeeld van een door Fannst opgesteld bericht is:
```
X-Mailer: FSMTP-V2
X-Fannst-NodeID: LUKERIEFF_MCLUST_A001
X-Made-By: Luke A.C.A. Rieff
Subject: SMTP Delivery failure
Date: Tue, 8 Sep 2020 14:20:37 (CEST)
MIME-Version: 1.0
Message-ID: <LO8A0Uf7mzsbS9eFKyo2FzizH8keV7LyOzpFQGj2yByo81uvI7UUxfLLep7sdrYJ@mailer.fannst.nl>
To: <webmaster@fannst.nl>
From: "Delivery Subsystem" <delivery@fannst.nl>
Content-Type: multipart/alternative; boundary="--mgHJ4i1Kmxeg5opqK0TXYgoW90ybb5EWdVePQB"
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=fannst.nl; s=default; 
       h=subject:date:mime-version:message-id:to:from; 
       bh=Gx7LgY7L9MRgrw9s8WI6K4h7LGqPBK2yd4CIF82NRUk=; b=p2nWDl0JzWGrcCydTmgY8SKjs7SOjEcop0JEDe7
        PWNk4kToS75J3lABaBAjujn6f7gvjrlxjM5y6Q6Y5CZqJNJQTApQEUpADrwK56dztLDFLOy5/UuBeGTaZGqkdyrYdJ
        S3TDKVWYBMXDHuILUsIUPh6+sauq1TG+ycnzV5HpcNdsS9Dg0GQ05o+mnywC9Wrd58YWpYZrmfnSgAWHBQzuq7Y2dc
        vCb+DKnhfNiTJ5JpBqxCBnoyIOU0oYJq30gW89+H+mDiKpzQSeRnvTUq/ZDUql/gUAQC7Jr+f3ccqRpniKGbjHfrzV
        1OpIxrknoPXcV13K8tYp6LApyNyx4NAMQ==
X-Fannst-Conn: SSL
X-Fannst-Auth: spf=pass (address 193.164.131.8 is listed by fannst.nl); 
       su=pass (access granted for 193.164.131.8 , using FSMTP-V2 extensions); 
       dkim=pass(signature[0]: is valid)


----mgHJ4i1Kmxeg5opqK0TXYgoW90ybb5EWdVePQB
Content-Type: text/html; charset="utf-8"
Content-Transfer-Encoding: quoted-printable

<=21DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://ww=
w.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns=3D"http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv=3D"Content-Type" content=3D"text/html; charset=3DUTF-8" />
<title>SMTP Delivery failure</title>
<meta name=3D"viewport" content=3D"width=3Ddevice-width, initial-scale=3D1.=
0"/>
<style type=3D"text/css">
body,html{margin:0;padding:0;font-family:sans-serif;}
/*Header*/
.wrap{padding:1rem;}
.hdr{background: rgba(19,60,75,1);
background: -moz-linear-gradient(left, rgba(19,60,75,1) 0%, rgba(14,94,122,=
1) 50%, rgba(21,129,163,1) 51%, rgba(19,60,75,1) 100%);
background: -webkit-gradient(left top, right top, color-stop(0%, rgba(19,60=
,75,1)), color-stop(50%, rgba(14,94,122,1)), color-stop(51%, rgba(21,129,16=
3,1)), color-stop(100%, rgba(19,60,75,1)));
background: -webkit-linear-gradient(left, rgba(19,60,75,1) 0%, rgba(14,94,1=
22,1) 50%, rgba(21,129,163,1) 51%, rgba(19,60,75,1) 100%);
background: -o-linear-gradient(left, rgba(19,60,75,1) 0%, rgba(14,94,122,1)=
 50%, rgba(21,129,163,1) 51%, rgba(19,60,75,1) 100%);
background: -ms-linear-gradient(left, rgba(19,60,75,1) 0%, rgba(14,94,122,1=
) 50%, rgba(21,129,163,1) 51%, rgba(19,60,75,1) 100%);
background: linear-gradient(to right, rgba(19,60,75,1) 0%, rgba(14,94,122,1=
) 50%, rgba(21,129,163,1) 51%, rgba(19,60,75,1) 100%);
filter: progid:DXImageTransform.Microsoft.gradient( startColorstr=3D'#133c4=
b', endColorstr=3D'#133c4b', GradientType=3D1 );
display:flex;align-items:center;justify-content:center;padding:0.9rem;borde=
r-bottom:0.4rem solid #1b1b1b;}
/*Footer*/
.ftr {background: rgba(32,34,36,1);
background: -moz-linear-gradient(left, rgba(32,34,36,1) 0%, rgba(34,37,38,1=
) 50%, rgba(37,40,41,1) 51%, rgba(25,26,27,1) 100%);
background: -webkit-gradient(left top, right top, color-stop(0%, rgba(32,34=
,36,1)), color-stop(50%, rgba(34,37,38,1)), color-stop(51%, rgba(37,40,41,1=
)), color-stop(100%, rgba(25,26,27,1)));
background: -webkit-linear-gradient(left, rgba(32,34,36,1) 0%, rgba(34,37,3=
8,1) 50%, rgba(37,40,41,1) 51%, rgba(25,26,27,1) 100%);
background: -o-linear-gradient(left, rgba(32,34,36,1) 0%, rgba(34,37,38,1) =
50%, rgba(37,40,41,1) 51%, rgba(25,26,27,1) 100%);
background: -ms-linear-gradient(left, rgba(32,34,36,1) 0%, rgba(34,37,38,1)=
 50%, rgba(37,40,41,1) 51%, rgba(25,26,27,1) 100%);
background: linear-gradient(to right, rgba(32,34,36,1) 0%, rgba(34,37,38,1)=
 50%, rgba(37,40,41,1) 51%, rgba(25,26,27,1) 100%);
filter: progid:DXImageTransform.Microsoft.gradient( startColorstr=3D'#20222=
4', endColorstr=3D'#191a1b', GradientType=3D1);
border-top: 1px solid #323232;
padding: 0.8rem;}
.ftr__ctr {text-align:center;}
.ftr__ctr-txt{color:#fff;font-size: 0.9rem;}
.ftr__ctr-txt>small{font-size:0.8rem;color:rgb(134, 134, 134)}
.ftr__ctr-txt>small>a,.ftr__ctr-txt>a{color: #0875a7;text-decoration:none;}
.ftr__ctr-txt>small>a:hover,.ftr__ctr-txt>a:hover{text-decoration:underline=
;}
.ftr__ctr-ref>ul>li{display: inline-block;}
.ftr__ctr-ref>ul>li:not(:last-child){margin-right: 0.4rem;}
.ftr__ctr-ref>ul>li>a{display: block;background-color:#ddd;
padding:0.3rem;color:#454545;text-decoration:none;border-radius:0.3rem;}
.ftr__ctr>hr{border:none;border-bottom:1px solid #474747;}
</style>
<style>
.errl{padding:0;margin:0}
.errl>li{list-style:none;}
.errl>li{background-color: #f0f0f0;border:1px solid #c7c7c7;border-radius:0=
.2rem;padding:0.8rem;display:block;}
.errl>li{margin-bottom:0.5rem;}
.errl>li>p{margin:0;padding:0;margin-bottom:1rem;}
.errl>li>p>strong{color:#0875a7;font-size:1rem;font-weight:bold;}
.errl>li>em{padding:0.5rem;border-radius:0.2rem;background-color:#1b1b1b;co=
lor:#88ff00;text-align: left;font-size: 0.9rem;}
</style>
</head>
<body>
<div class=3D"wrap">
<header class=3D"hdr">
<img src=3D"https://software.fannst.nl/fannst.nl/assets/brand/fannst-banner=
-light.png" width=3D"320px" />
</header>
<div class=3D"wrap__ctnt">
<h1>Delivery failure</h1>
<p>The FSMTP-V2 Nodes failed to deliver your message.</p>
<p>For the geeks out there, we've logged the events bellow.</p>
<hr />
<ul class=3D"errl">
 =
<li>
<p><strong>&lt;asdasdasdasdasdasdasddasdasd@gmail.com&gt;</strong>: </p>
<em>Transmission error: 550: The email account that you tried to reach does=
 not exist. Please try</em>
</li>
 =
</ul>
</div>
<footer class=3D"ftr">
<div class=3D"ftr__ctr">
<p class=3D"ftr__ctr-txt">
Sent using FSMTP-V2 by <a href=3D"https://me.fannst.nl">Luke A.C.A. Rieff</=
a>
<br />
<small>For any further inquiries, send message to: <a href=3D"mailto:lrieff=
@fannst.nl">lrieff@fannst.nl</a></small>
</p>
<hr />
<div class=3D"ftr__ctr-ref">
<ul>
<li>
<a href=3D"https://github.com/skywa04885/fsmtp-v2">Github</a>
</li>
<li>
<a href=3D"https://software.fannst.nl">Fannst Software</a>
</li>
</ul>
</div>
</div>
</footer>
</div>
</body>
</html>

----mgHJ4i1Kmxeg5opqK0TXYgoW90ybb5EWdVePQB--
```

Een POP3 server gesprek:

```bash
+OK POP3 server ready LUKERIEFF_MCLUST_A001
user jan
+OK Send PASS
pass jan123
+OK Auth Success
pass
-ERR Order Error: Client already authenticated.
uidl
+OK UIDL follows
1 1595169235894
2 1595162385699
3 1595162175780
4 1595158783138
5 1595155108162
6 1595092185618
7 1595091526717
.
list
+OK LIST follows
1 1241
2 2243
3 448
4 3224
5 2239
6 2233
7 2292
.
retr 2
+OK 2243 octets
Received: by mail-pj1-f47.google.com with SMTP id t15so8629004pjq.5        for <jan@fannst.nl>; Sun, 19 Jul 2020 05:39:45 -0700 (PDT)
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=gmail.com; s=20161025; h=mime-version:from:date:message-id:subject:to; bh=kEHqC/dEDwOikTvj37xshDYcWWYP4MX4/Vb/jDsTL6w=; b=uuaopL2aCVilZ/HPQVRgn28afNBtza8v4+P112qYQcgmlqBUjjlo3k4E8BktZTABI6Sm2V+4L9YAGKomdpebWVAZYQhaKBSj4LerRj+S0oo579RuP2vSz/vDSrDkPe8kIYRjd7IQjNp6B6tWKapUsuJuHBLusch8zMUE7bT9QRev2E/qMmpYBpcfDqCvNQ0hEPrcwzsgHPsMl+DNy81IbGvOi1M0tCCM8TKhT1oU2I+0T3nhSq2CA2qJg/2O/5yt9dwJQ/eEeZWiOu0UZHM7qPIfiAFPu1pnKiy+pUYYjEStBhhds8uB9kQLKbvtTDmdli1fx3yOHa6oQiRHzStaXw==
X-Google-DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=1e100.net; s=20161025; h=x-gm-message-state:mime-version:from:date:message-id:subject:to; bh=kEHqC/dEDwOikTvj37xshDYcWWYP4MX4/Vb/jDsTL6w=; b=t2GXDGDIl+SHZW6U/35LiiBUk2DXPQEmEZlp8PcSnzGOt4H30atCn4H3QI+D26JAOLzr2JYSuWt3m/iw7vE8GkxENwtsaqH4fs/uWcTw0c2bFdie67+WCRIfbC4Q8eI2JVraUv+hZ91XtDi1eXckUHr/iLhpiDUP1fg9MOnjww6WN7ih/slpvcgjbhlrnyGneCbmJAvp25rxATnwJYaQm+dwC/CINHU+ZFVq0mSVyjyyXr2/2Ajt4rr7IA77ornPxlYpWrHLsso+A         fmrTTunUKNBPoqwKEnbxu53ild8RZOYdcrkw+Mfg5TvqSXzpiZVADzq12noDizEOci5aCvgg==
X-Gm-Message-State: AOAM533g7kX5+qrRzEQxsduBUZyJvYF6ZCw0nYgmv4FOtW50aSU7h7/W	+HembzZO+pG8ecLZiR5Tyc+vcdbMrq56pRK95SmSAFvP
X-Google-Smtp-Source: ABdhPJz6n82rs1QnhtgLxrql1k/j2qZ0AjDav6MTAd3kiZl45VkGzRl44MJKSV8lWPfjf9d3Mf2bs9yzdC2nQ6i5h1c=
X-Received: by 2002:a17:90a:ff92:: with SMTP id hf18mr18670796pjb.10.1595162383461; Sun, 19 Jul 2020 05:39:43 -0700 (PDT)
MIME-Version: 1.0
From: Skywalker <luke.rieff@gmail.com>
Date: Sun, 19 Jul 2020 14:39:31 +0200
Message-ID: <CAFoEhQPaEv60jy+Rzms=Z4wnrEM_9tUSvUz9bPr-_QfFYL3Gig@mail.gmail.com>
Subject: Hoi
To: jan@fannst.nl
Content-Type: multipart/alternative; boundary="00000000000039b14705aacaafb3"

--00000000000039b14705aacaafb3
Content-Type: text/plain; charset="UTF-8"

Shhzshhs

--00000000000039b14705aacaafb3
Content-Type: text/html; charset="UTF-8"

<div dir="auto">Shhzshhs</div>

--00000000000039b14705aacaafb3--
.
quit
+OK POP3 server signing off LUKERIEFF_MCLUST_A001
```

## Auteurs
|Name|Github|
|-|-|
|Luke Rieff|[Click here](https://github.com/skywa04885)|

**Python sucks**