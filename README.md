SIPbot
===

SIPbot is a VoIP answering machine written in C.
It's easy to use, SIPbot answers VoIP calls and each call is driven by a custom program or script.

Requirements
---

If you want to build SIPbot, you need these libraries:
- eXosip2 (4.0.0)
- osip2 (4.0.0)
- oRTP (0.22.0)

Otherwise, if you just want to run it, you need only a VoIP account.
I use a free phone number given by messagenet.it

Usage
---

#### 0) Get a VoIP Account

You can get a free VoIP account on messagenet.com

I've tried only that one. Let me know if SIPbot does not work with other services.

#### 1) Download libraries and compilers

    $ sudo apt-get install build-essential libexosip2-dev libortp-dev libosip2-dev

#### 2) Clone

    $ git clone https://github.com/carpikes/sipbot.git
    $ cd sipbot

#### 3) Build

    $ make

If you read some library error, check their version!

#### 4) Configure

    $ cp template.config sipbot.ini
    $ vim sipbot.ini # or open with your favourite editor

`sipbot.ini` is well documented :)

Write your account info inside that file.

Note that `program_name` macro contains the name of the program 
which will be launched on each received call.

#### 5) Launch

    $ ./sipbot
    
Sipbot will try to load the default config file `sipbot.ini`.
If you want to specify a custom config file, use the `-c` parameter,
e.g. `./sipbot -c custom.ini`

Now when you call your VoIP number, SIPbot will launch your program and handle the call.

Reload config
---

If you want to reload the configuration, send a `SIGHUP` to sipbot.

    $ kill -HUP `pidof sipbot`

Program protocol
---

SIPbot communicates in a very simple way with the launched program, via stdin and stdout.
`scripts/test.php` is a simple example.

#### Commands sent from SIPbot to your program
- When a call is received, SIPbot launch the program and write into his stdin `CALL "+12345"\n`, where `+12345` is the calling phone number, or `Anonymous` if the caller is unknown.

- When a user press a number on his phone, `DTMF N\n` is sent to the program, where `N` is the key pressed.

- When the call is closed, `FNSH\n` is sent to the program.

#### Commands sent from your program to SIPbot

Your program can send the following commands to SIPbot (via stdout):
- `PLAY filename\n`, SIPbot plays `filename`.
- `APND filename\n`, SIPbot queues `filename` after the current playing file.
- `STOP\n`, SIPbot stops playing and clears the queue
- `KILL\n`, SIPbot closes the call

Audio files
---

Actually SIPbot plays only *mono* (1 audio channel) WAV files

You can convert a song in this format using ffmpeg
```
$ ffmpeg -i input_audio.mp3 -ar 44100 -ac 1 output_audio.wav
```

License
---

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
