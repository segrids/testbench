/* MIT License

Copyright (c) 2021 by SEGRIDS GmbH <www.segrids.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 

Author: Frank Schuhmacher <frank.schuhmacher@segrids.com>
*/
 
/** APDU handler 

The command interface is defined by a simplified APDU structure. 

 
Simple APDU format:

Field	| len  | interpretation
------- | ---- | --------------------
INS		|	1  |  instruction identifier
LC		|	2  |  size of DATA field
DATA	|	LC |  payload data
LE		|	2  |  expected response len
 
A maximal DATA length LC of 260 is admitted. 


Respose format:

Field	| len
------- | ------------
DATA	|  LE
  SW	|  2

Note that LE only refers to the response DATA field.
SW = 0x9000 indicates a correct command execution.
If LC > 260, error status SW = 0x6700 is returned.
If an instruction INS is not supported, the error status SW=0x6D00 is returned.
*/

#include "apdu.h"
#include "types.h"
#include "usart.h"

apdu_t apdu;
