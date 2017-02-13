# Quirc

QR codes are a type of high-density matrix barcodes, and quirc is a
library for extracting and decoding them from images. It has several
features which make it a good choice for this purpose:

  * It is fast enough to be used with realtime video: extracting and
    decoding from VGA frame takes about 50 ms on a modern x86 core.

  * It has a robust and tolerant recognition algorithm. It can
    correctly recognise and decode QR codes which are rotated and/or
    oblique to the camera. It can also distinguish and decode multiple
    codes within the same image.

  * It is easy to use, with a simple API described in a single
    commented header file.

  * It is small and easily embeddable, with no dependencies other than
    standard C functions.

  * It has a very small memory footprint: one byte per image pixel,
    plus a few kB per decoder object.

  * It uses no global mutable state, and is safe to use in a
    multithreaded application.

  * BSD-licensed, with almost no restrictions regarding use and/or
    modification.

##License

https://github.com/dlbeer/quirc

Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
above copyright notice and this permission notice appear in all
copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.