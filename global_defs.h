/*
 Copyright(c) 2015 Graham Chow

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 * global_defs.h
 *
 *  Created on: 24 Oct 2015
 *      Author: graham
 */

#ifndef GLOBAL_DEFS_H_
#define GLOBAL_DEFS_H_

#define UI_CAPS_LOCK            0x00000001
#define UI_SCROLL_LOCK          0x00000002
#define UI_NUM_LOCK             0x00000003

#define ADC_BUFFER_SIZE (4096)
#define HW_AVERAGE (64)
#define NUM_CHANNELS (8)
#define FFT_BIN_SIZE (1024)

#define FILTER_SIZE (145+3)
#define FILTER_BUFFER_ALLOWANCE (8)
#define FILTER_BUFFER_SIZE (FILTER_SIZE + FILTER_BUFFER_ALLOWANCE)
#define FILTER_STEP_SIZE (FILTER_SIZE/2)

#define MULTIPLY_DOWN_SHIFT (8)
#define THRESHOLD_SHIFT (1)

#define MESSAGE_LENGTH (2)

#endif /* GLOBAL_DEFS_H_ */
