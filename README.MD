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
sudo apt-get install meson ninja
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
Een voorbeeld van een door Fannst opgesteld bericht is:
```
X-Mailer: FSMTP-V2
X-Fannst-NodeID: FC_EU_N0001
X-Made-By: Luke A.C.A. Rieff
Subject: Hello
Date: Mon, 13 Jul 2020 22:10:10 CEST
MIME-Version: 1.0
Message-ID: <6HgYov0Etosg20nhf3iQP3yyOMwi4VuvferEupE2eGoMEclTKFkuLjc3Gy3uChbR@mail.fannst.nl>
To: "Example" <test@gmail.com>
From: "Test" <test@fannst.nl>
Content-Type: multipart/alternative; boundary="--EOdZuGKWSdoWiR0cSAzXom4Wv3XeluGcCVid06"
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed; d=fannst.nl; s=default;
       h=subject:date:mime-version:message-id:to:from;
       bh=1r6joMzD65Y2I+kDL40gRc/oD2qi9fzYx4IcpkYniQw=; b=GMkrL9n0FVr87NRTSTksp3qs/FO6gU4UTLp7QP1
        Qxx3rGJD41aVpEFq7ZKHCcqO2b5k3QjbwaOwUeZfL8p5sksqnE2TXLbUA5iMfUr8vH/AtMw5E+7uH9Cn5Ye8JbZWXa
        f1jL2iC5BnrU4DSUtl2+XbhRBfsWkhiOA9pgD35azye/jUoXetG8ulw/bdUb8DWulUgGd9oRZybFpe2s54yao9dG0q
        /wSgn5h5GdSPE78uLwGGj/q14x9j1xHDW7EdXUHtkKWN9DtrRhI3L6da/TnYlW16AeR4nK+LQOaxaG9+qAKylJkaYK
        +70O+DuNwEumJP1Yhbyh/TS2+cf1zRdGw==

----EOdZuGKWSdoWiR0cSAzXom4Wv3XeluGcCVid06
Content-Type: text/html; charset="utf-8"
Content-Transfer-Encoding: quoted-printable

<p>
<strong>Roses are red.</strong><br />
<small>Violets are blue.</small><br />
<em>Someone did NOT specify an message body.</em></p>

----EOdZuGKWSdoWiR0cSAzXom4Wv3XeluGcCVid06
Content-Type: text/plain; charset="utf-8"
Content-Transfer-Encoding: quoted-printable

Roses are red.
Violets are blue.
Someone did NOT specify an message body.

----EOdZuGKWSdoWiR0cSAzXom4Wv3XeluGcCVid06--
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