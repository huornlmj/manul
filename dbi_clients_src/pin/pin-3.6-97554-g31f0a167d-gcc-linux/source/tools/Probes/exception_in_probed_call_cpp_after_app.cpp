/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
  This application causes exception in indirect call instruction and catches it in caller.
  The call instruction is located in code region being replaced by Pin probe.
  Pin translation should not affect propagation of the exception to the C++ exception handler.
*/
#ifdef TARGET_WINDOWS
#include <windows.h>
#endif
#include <stdio.h>

bool destructed = false;

// cpp exceptions - Exercise windows exception mechanism 
class MyClass
{
public:
    ~MyClass() 
    { 
        destructed = true;
    }
};


#ifdef TARGET_WINDOWS
#define FASTCALL __fastcall
#define DLLEXPORT __declspec(dllexport)
#else
#define FASTCALL 
#define DLLEXPORT 
#endif

static int (FASTCALL *pBar)(int, int, int, int, int, int, int) = 0;

int FASTCALL bar(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    return a0 + a1 + a2 + a3 + a4 + a5 + a6;
}

extern "C"
DLLEXPORT
int FASTCALL foo(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
#if defined(TARGET_LINUX) || defined(TARGET_MAC)
    if (!pBar) throw(0);
#endif
    // May cause exception due to NULL pointer
    return pBar(a0, a1, a2, a3, a4, a5, a6);
    
}


int main()
{
    int i = 2;
    int local = 1;

    try
    {
        MyClass ins;
        i = foo(1, 2, 3, 4, 5, 6, 7);
        local = 0;
    }
    catch(...)
    {
        // If Pin translated probed code properly, exception will reach the handler
        printf("Exception\n");
    }

    // Check that destructor was called and local var value was not changed when exception was handled
    if (!destructed || (local != 1))
    {
        return 1;
    }

    pBar = bar;

    try
    {
        i = foo(1, 2, 3, 4, 5, 6, 7);
    }
    catch(...)
    {
        // No exception expected
        printf("Exception\n");
    }

    printf("result=%d\n", i);

    return 0;
}
