#SIPbot#
###An opensource VoIP answering machine###

---
SIPbot is a VoIP answering machine written in C.

Actually it answers VoIP calls and plays an audio stream.

---
###Notes###
SIPbot is still **under development**.

If you are looking for a complete VoIP answering machine, come back in a few months.

---
###Requirements###
If you want to build SIPbot, you need these libraries:
- eXosip2 (4.0.0)
- osip2 (4.0.0)
- oRTP (0.22.0)

Otherwise, if you just want to run it, you need only a VoIP account.
I use a free phone number given by messagenet.it

---
###Usage###
- First of all, you need libraries. Download and install them.
- Now you need a VoIP account. I've tried only messagenet.it. Let me know if this program does not work with other services.
- Now write your VoIP account login into config.h
- Finally you are ready to build this source. Type:
```
$ make
```
If you read some library error, check their version!
- Try to launch it with:
```
$ ./sipbot
```
and when you call your VoIP number, SIPbot will stream music.wav over the net.

---
###How can I change that music?###
Actually SIPbot reads music.wav, it is a RIFF file with these specs:
- Wave
- Compressed with muLaw
- Audio rate: 8000
- Audio channels: 1 (mono)

You can convert a song in this format using ffmpeg
```
$ ffmpeg -i your_song.mp3 -acodec pcm_mulaw -ar 8000 -ac 1 music.wav
```

---
###License###
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