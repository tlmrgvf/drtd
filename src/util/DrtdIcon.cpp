/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* GIMP RGB C-Source image dump (drtd.c) */

static const struct {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[32 * 32 * 3 + 1];
} drtd_icon_data = {
    32,
    32,
    3,
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\302\000\000\302\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\302\000\000\302\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\306\000\000\306\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\265\000\000\377\000\000\377\000\000\265\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\261\000\000\377\000\000\377\000\000\261\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\261\000"
    "\000\377\000\000\377\000\000\261\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\003\000\000\000\000\000\257\000\000\377\000\000\377\000"
    "\000\257\000\000\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\063\000\000\305\000\000\377\000\000\377\000\000\305\000\000\063\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\260\000\000\377\000\000\376\000\000\376\000\000\377\000\000\260\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\252\000\000\377\000\000\377\000\000\377\000\000\377\000\000\252\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\005\000\000\000\000\000\245\000\000\377"
    "\000\000\377\000\000\377\000\000\377\000\000\245\000\000\000\000\000\005\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\005\000\000\000\000\000D\000\000\317\000\000\377\000\000\377\000\000\377"
    "\000\000\377\000\000\317\000\000D\000\000\000\000\000\005\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\003\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000D\000\000\324\000\000\377\000\000\376\000\000\377\000\000\377\000\000\376\000\000\377"
    "\000\000\324\000\000D\000\000\000\000\000\000\000\000\000\000\000\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\063\000\000\260\000\000\252\000"
    "\000\245\000\000\317\000\000\377\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376\000"
    "\000\377\000\000\317\000\000\245\000\000\252\000\000\260\000\000\063\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\265\000\000\261\000\000\261\000\000\257\000\000\305"
    "\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377"
    "\000\000\377\000\000\377\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377\000\000\305\000\000\257"
    "\000\000\261\000\000\261\000\000\265\000\000\000\000\000\000\000\000\000\000\000\302\000\000\302\000\000\306\000\000\377"
    "\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377"
    "\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377"
    "\000\000\377\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\306\000\000\302"
    "\000\000\302\000\000\302\000\000\302\000\000\306\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377"
    "\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377"
    "\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376\000\000\377\000\000\377"
    "\000\000\377\000\000\377\000\000\377\000\000\306\000\000\302\000\000\302\000\000\000\000\000\000\000\000\000\000\000\265"
    "\000\000\261\000\000\261\000\000\257\000\000\305\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376"
    "\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376\000\000\377\000\000\377"
    "\000\000\377\000\000\377\000\000\305\000\000\257\000\000\261\000\000\261\000\000\265\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\063\000\000\260\000\000\252\000\000\245"
    "\000\000\317\000\000\377\000\000\376\000\000\377\000\000\377\000\000\377\000\000\377\000\000\376\000\000\377"
    "\000\000\317\000\000\245\000\000\252\000\000\260\000\000\063\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000D\000\000\324\000\000\377\000\000\376\000\000\377\000\000\377\000\000\376\000\000\377\000\000\324\000"
    "\000D\000\000\000\000\000\000\000\000\000\000\000\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\005\000\000\000\000\000"
    "D\000\000\317\000\000\377\000\000\377\000\000\377\000\000\377\000\000\317\000\000D\000\000\000\000\000\005\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\005\000\000\000\000\000\245\000\000"
    "\377\000\000\377\000\000\377\000\000\377\000\000\245\000\000\000\000\000\005\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\252\000\000\377\000\000\377"
    "\000\000\377\000\000\377\000\000\252\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\260\000\000\377\000\000\376\000\000\376\000\000"
    "\377\000\000\260\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\063\000\000\305\000\000\377\000\000\377\000\000\305\000\000\063\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\003\000\000\000\000\000\257\000\000\377\000\000\377\000\000\257\000\000\000\000\000\003\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\261\000\000\377\000\000\377\000\000\261\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\261\000\000\377"
    "\000\000\377\000\000\261\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\265\000\000\377\000\000\377\000\000\265"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\306\000\000\306\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\302\000\000\302\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\302\000"
    "\000\302\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000",
};