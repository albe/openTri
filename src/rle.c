/*
	RLE encoder/decoder for 4,8,16,24 and 32bit
	written by alexander berl, 2004/2007
*/
#ifndef __RLE_C__
#define __RLE_C__

#include "rle.h"


void decodeRLE( void* src, int srcLen, void* dst, int dstLen, int bits )
{
    #ifndef assembler_decode__
    long count = 0;
    long count2 = 0;
    unsigned char* s = (unsigned char*)src;
    unsigned char* d = (unsigned char*)dst;
    if (bits==4) {
      int sswap = 0, dswap = 0;
      do {
       unsigned char c;
       if (sswap)
		 c = (*(s+count) & 0xF0) | (*(s+count+1) & 0xF);
	   else
	     c = *(s+count);
	   count++;
       if (c & 0x80) {
         unsigned char a;
         if (sswap^=1)
           a = (*(s+count) & 0xF);
         else
           a = (*(s+count++) >> 4);
         c = (c & 0x7F)+1;
         if (dswap)
         {
           *(d+count2++) |= (a << 4);
           c--;
           dswap = 0;
         }
         for (;c>1;c-=2)
           *(d+count2++) = (a | a << 4);
         if (c)
         {
           dswap = 1;
           *(d+count2) = a;
         }
       } else {
         c++;
         if (sswap == dswap)
         {
           if (sswap)
           {
             *(d+count2++) |= *(s+count++) & 0xF0;
             c--;
           }
		   for (;c>1;c-=2)
			 *(d+count2++) = *(s+count++);
		   if (c)
		   {
		     *(d+count2) = *(s+count) & 0xF;
		     dswap = sswap = 1;
		   }
		 }
		 else
		 {
		   for (;c>0;c--)
		     if (dswap^=1)
		       *(d+count2) = *(s+count++) >> 4;
		     else
		       *(d+count2++) |= *(s+count) << 4;
		   sswap = dswap^1;
		 }
       }
      } while ((count<srcLen) && (count2<dstLen));
 
 
    } else if (bits==8) {
      do {
       unsigned char c = *(s+count++);
       if (c & 0x80) {
         c = (c & 0x7F);
         unsigned char a = *(s+count++);
         for (c++;c>0;c--)
           *(d+count2++) = a;
       } else {
         for (c++;c>0;c--)
           *(d+count2++) = *(s+count++);
       }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==16) {
      do {
       unsigned char c = *(s+count++);
       if (c & 0x80) {
		 unsigned char b1 = *(s+count++);
		 unsigned char b2 = *(s+count++);
         c = (c & 0x7F);
         for (c++;c>0;c--) {
           *(d+count2++) = b1;
           *(d+count2++) = b2;
         }
       } else {
         for (c++;c>0;c--) {
           *(d+count2++) = *(s+count++);
           *(d+count2++) = *(s+count++);
         }
       }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==24) {
      do {
       unsigned char c = *(s+count++);
       if (c & 0x80) {
         unsigned char t[3];
         t[0] = *(s+count++);
         t[1] = *(s+count++);
         t[2] = *(s+count++);
         c = (c & 0x7F);
         for (c++;c>0;c--) {
           *(d+count2++) = t[0];
           *(d+count2++) = t[1];
           *(d+count2++) = t[2];
         }
       } else {
         for (c++;c>0;c--) {
           *(d+count2++) = *(s+count++);
           *(d+count2++) = *(s+count++);
           *(d+count2++) = *(s+count++);
         }
       }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==32) {
      do {
       unsigned char c = *(s+count++);
       if (c & 0x80) {
	     unsigned char w[4];
         w[0] = *(s+count++);
         w[1] = *(s+count++);
         w[2] = *(s+count++);
         w[3] = *(s+count++);
         c = (c & 0x7F);
         for (c++;c>0;c--) {
           *(d+count2++) = w[0];
           *(d+count2++) = w[1];
           *(d+count2++) = w[2];
           *(d+count2++) = w[3];
         }
       } else {
         for (c++;c>0;c--) {
           *(d+count2++) = *(s+count++);
           *(d+count2++) = *(s+count++);
           *(d+count2++) = *(s+count++);
           *(d+count2++) = *(s+count++);
         }
       }
      } while ((count<srcLen) && (count2<dstLen));
    }
    
    #else
    #ifdef PSP
    if (bits==8) {
		asm(
			"addiu $8, $0, $0\n"		// edx = 0
			"__loop8:\n"
			"addiu $9, $0, $0\n"		// eax = 0
			"addiu $8, $8, 1\n"			// edx += 1
			
			"lb $10, (%1)\n"			// lb al
			"addiu %1, %1, 1\n"
			
			"and $12, $10, 0x80\n"
			"cmp EZ, $12\n"
			"bt __lower8f\n"
			
			"addiu $10, $10, 1\n"
			"move $11, $10\n"
			"addiu $9, $9, $10\n"
			"__copy8:\n"
			"lb $10, (%1)\n"
			"addiu %1, %1, 1\n"
			"sb $10, (%0)\n"
			"addiu %0, %0, 1\n"
			"addiu $11, $11, -1\n"
			"cmp EZ, $11\n"
			"bf __copy8b\n"
			
			"cmp BT,$9, %2\n"
			"bt __loop8b\n"
			"b __done8f\n"
			
			"__lower8:\n"
			"and $10, $10, 127\n"
		
		:"=r"(dst):"r"(src),"r"(srcLen),"r"(dstLen));
	#else
	// only on x86-architecture
    if (bits==8) {
		__asm{
                        mov esi, src
                        mov edi, dst
                        mov ebx, srcLen
                        xor edx, edx

                      __loop8:
                        xor eax, eax
                        inc edx
                        lodsb
                        or al, al
                        js __lower8
                        inc eax
                        mov ecx, eax
                        add edx, eax
                        rep movsb
                        cmp edx, ebx
                        jb __loop8
                        jmp __done8

                      __lower8:
                         and al, 127
                         inc eax
                         mov ecx, eax
                         inc edx
                         lodsb
                         rep stosb

                         cmp edx, ebx
                         jb __loop8
                         //jmp __done8

                       __done8:
		}
      } else if (bits==16) {
                  __asm{
                        mov esi, src
                        mov edi, dst
                        mov ebx, srcLen
                        xor edx, edx

                      __loop16:
                        xor eax, eax
                        inc edx
                        lodsb
                        or al, al
                        js __lower16
                        inc eax
                        mov ecx, eax
                        shl eax, 1
                        add edx, eax
                        rep movsw
                        cmp edx, ebx
                        jb __loop16
                        jmp __done16

                      __lower16:
                         and al, 127
                         inc eax
                         mov ecx, eax
                         add edx, 2
                         lodsw
                         rep stosw

                         cmp edx, ebx
                         jb __loop16
                         //jmp __done16

                       __done16:
                  }
     } else if (bits==24) {
                 __asm {
                        mov esi, src
                        mov edi, dst
                        mov ebx, srcLen
                        xor edx, edx

                      __loop24:
                        xor eax, eax
                        inc edx
                        lodsb
                        or al, al
                        js __lower24

                        inc eax
                        mov ecx, eax
                        add edx, eax
                        lea edx, [edx+eax*2]
                      __loop124:
                        mov ax, [esi]
                        mov [edi], ax
                        mov al, [esi+2]
                        mov [edi+2], al
                        add esi, 3
                        add edi, 3
                        dec ecx
                        jnz __loop124

                        cmp edx, ebx
                        jb __loop24
                        jmp __done24

                      __lower24:
                        and al, 127
                        inc eax
                        mov ecx, eax
                        add edx, 3
                        mov ax, [esi]
                        shl eax, 16
                        mov al, [esi+2]
                      __loop224:
                        ror eax, 16
                        mov [edi], ax
                        ror eax, 16
                        mov [edi+2], al
                        add esi, 3
                        add edi, 3
                        dec ecx
                        jnz __loop224

                        cmp edx, ebx
                        jb __loop24
                        //jmp __done24

                      __done24:
                      }
     } else if (bits==32) {
                __asm{
                        mov esi, src
                        mov edi, dst
                        mov ebx, srcLen
                        xor edx, edx

                      __loop32:
                        xor eax, eax
                        inc edx
                        lodsb
                        or al, al
                        js __lower32
                        inc eax
                        mov ecx, eax
                        shl eax, 2
                        add edx, eax
                        rep movsd
                        cmp edx, ebx
                        jb __loop32
                        jmp __done32

                      __lower32:
                         and al, 127
                         inc eax
                         mov ecx, eax
                         add edx, 2
                         lodsd
                         rep stosd

                         cmp edx, ebx
                         jb __loop32
                         //jmp __done32
                         
                       __done32:
                  }
       }
       #endif
       #endif
}


// returns RLE encoded size of data, linesz gives number of pixels after which to stop any runs
long encodeRLE( void* src, int srcLen, int linesz, void* dst, int dstLen, int bits )
{
	if (linesz==0) linesz = (srcLen << 3) / (bits);
    int i;
    long count = 0;
    long count2 = 0;
    unsigned char* s = (unsigned char*)src;
    unsigned char* d = (unsigned char*)dst;
    if (bits==4) {
      unsigned char sswap = 0, dswap = 0;
      do {
       unsigned char maxrun = (linesz-((count*2)%linesz));
       if (maxrun>(srcLen-count)*2) maxrun = (srcLen-count)*2;
       if (maxrun>128) maxrun = 128;

       unsigned char run;
       unsigned char c;
       if (sswap)
       {
	     c = (*(s+count) >> 4) | (*(s+count+1) << 4);
	     count++;
	   }
	   else
         c = *(s+count);
       sswap ^= 1;
       if (count2>=dstLen) break;
       if (maxrun>=2 && (c >> 4)==(c & 0xF))
       {
		 for (run=1;run<maxrun && ((sswap)?(c & 0xF)==(*(s+count++)>>4):(c & 0xF)==(*(s+count)&0xF));run++, sswap^=1);
		 if (dswap^=1)
		 {
			 *(d+count2++) = 0x80 | (run-1);
			 *(d+count2) = c & 0xF;
		 }
		 else
		 {
			 *(d+count2++) |= (0x80 | (run-1)) & 0xF0;
			 *(d+count2++) = (c << 4) | ((0x80 | (run-1)) & 0xF);
		 }
	   }
	   else
	   {
	     unsigned char tswap = sswap ^ 1;
	     unsigned char tcount = count;
	     for (run=1;run<maxrun && ((sswap)?(*(s+count++)>>4)!=(*(s+count+1)&0xF):(*(s+count)>>4)!=(*(s+count)&0xF));run++, sswap^=1);
		 if (count2+run/2+1>=dstLen) break;
	     if (!dswap)
			*(d+count2++) = (0x00 | (run-1));
		 else
		 {
		    *(d+count2++) |= (run-1) & 0xF0;
		    *(d+count2) = (run-1) & 0xF;
		 }
		 if (dswap==tswap)
		 {
		    if (dswap)
		    {
		      *(d+count2++) |= (*(s+tcount) & 0xF0);
		      run--;
		      dswap = 0;
		    }
		    for (i=0;i<run/2;i++)
		      *(d+count2++) = *(s+tcount+i);
		    if (run&1)
		    {
		      *(d+count2) = *(s+count) & 0xF;
		      dswap = 1;
		    }
		  }
		  else
		  {
		    for (i=0;i<run;i++)
			  if (dswap^=1)
			    *(d+count2) = (*(s+tcount++) >> 4);
			  else
			    *(d+count2++) |= (*(s+tcount) << 4);
		  }
	   }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==8) {
      do {
       unsigned char maxrun = (linesz-(count%linesz));
       if (maxrun>srcLen-count) maxrun = srcLen-count;
       if (maxrun>128) maxrun = 128;
       unsigned char run;
       
       unsigned char c = *(s+count++);
       if (count2>=dstLen-1) break;
       if (maxrun>=2 && c==*(s+count))
       {
		 for (run=1;run<maxrun && c==*(s+count);run++,count++);
		 *(d+count2++) = (0x80 | (run-1));
	     *(d+count2++) = c;
	   }
	   else
	   {
	     for (run=1;run<maxrun && *(s+count)!=*(s+count+1);run++,count++);
	     *(d+count2++) = (0x00 | (run-1));
	     if (count2+run>=dstLen) break;
	     for (i=0;i<run;i++)
	      *(d+count2++) = *(s+count-run+i);
	   }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==16) {
      do {
       unsigned char maxrun = linesz-((count/2) % linesz);
       if (maxrun>(srcLen-count)/2) maxrun = (srcLen-count)/2;
       if (maxrun>128) maxrun = 128;
       unsigned char run;
       
       unsigned char c1 = *(s+count++);
       unsigned char c2 = *(s+count++);
       if (count2>=dstLen-2) break;
       if (maxrun>=2 && c1==*(s+count) && c2==*(s+count+1))
       {
		 for (run=1;run<maxrun && c1==*(s+count) && c2==*(s+count+1);run++,count+=2);
		 *(d+count2++) = (0x80 | (run-1));
	     *(d+count2++) = c1;
	     *(d+count2++) = c2;
	   }
	   else
	   {
	     for (run=1;run<maxrun && (*(s+count)!=*(s+count+2) || *(s+count+1)!=*(s+count+3));run++,count+=2);
	     *(d+count2++) = (0x00 | (run-1));
	     if (count2+run*2>=dstLen) break;
	     for (i=0;i<run;i++)
	     {
	       *(d+count2++) = *(s+count-run*2+i*2);
	       *(d+count2++) = *(s+count-run*2+i*2+1);
	     }
	   }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==24) {
      do {
       unsigned char maxrun = linesz-((count/3) % linesz);
       if (maxrun>(srcLen-count)/3) maxrun = (srcLen-count)/3;
       if (maxrun>128) maxrun = 128;
       unsigned char run;
       
       unsigned char c1 = *(s+count++);
       unsigned char c2 = *(s+count++);
       unsigned char c3 = *(s+count++);
       if (count2>=dstLen-3) break;
       if (maxrun>=2 && c1==*(s+count) && c2==*(s+count+1) && c3==*(s+count+2))
       {
		 for (run=1;run<maxrun && c1==*(s+count) && c2==*(s+count+1) && c3==*(s+count+2);run++,count+=3);
		 *(d+count2++) = (0x80 | (run-1));
	     *(d+count2++) = c1;
	     *(d+count2++) = c2;
	     *(d+count2++) = c3;
	   }
	   else
	   {
	     for (run=1;run<maxrun && (*(s+count)!=*(s+count+3) || *(s+count+1)!=*(s+count+4) || *(s+count+2)!=*(s+count+5));run++,count+=3);
	     *(d+count2++) = (0x00 | (run-1));
	     if (count2+run*3>=dstLen) break;
	     for (i=0;i<run;i++)
	     {
	       *(d+count2++) = *(s+count-run*3+i*3);
	       *(d+count2++) = *(s+count-run*3+i*3+1);
	       *(d+count2++) = *(s+count-run*3+i*3+2);
	     }
	   }
      } while ((count<srcLen) && (count2<dstLen));


    } else if (bits==32) {
      do {
       unsigned char maxrun = linesz-((count/4) % linesz);
       if (maxrun>(srcLen-count)/4) maxrun = (srcLen-count)/4;
       if (maxrun>128) maxrun = 128;
       unsigned char run;
       
       unsigned char c1 = *(s+count++);
       unsigned char c2 = *(s+count++);
       unsigned char c3 = *(s+count++);
       unsigned char c4 = *(s+count++);
       if (count2>=dstLen-4) break;
       if (maxrun>=2 && c1==*(s+count) && c2==*(s+count+1) && c3==*(s+count+2) && c4==*(s+count+3))
       {
		 for (run=1;run<maxrun && c1==*(s+count) && c2==*(s+count+1) && c3==*(s+count+2) && c4==*(s+count+3);run++,count+=4);
		 *(d+count2++) = (0x80 | (run-1));
	     *(d+count2++) = c1;
	     *(d+count2++) = c2;
	     *(d+count2++) = c3;
	     *(d+count2++) = c4;
	   }
	   else
	   {
	     for (run=1;run<maxrun && (*(s+count)!=*(s+count+4) || *(s+count+1)!=*(s+count+5) || *(s+count+2)!=*(s+count+6) || *(s+count+3)!=*(s+count+7));run++,count+=4);
	     *(d+count2++) = (0x00 | (run-1));
	     if (count2+run*4>=dstLen) break;
	     for (i=0;i<run;i++)
	     {
	       *(d+count2++) = *(s+count-run*4+i*4);
	       *(d+count2++) = *(s+count-run*4+i*4+1);
	       *(d+count2++) = *(s+count-run*4+i*4+2);
	       *(d+count2++) = *(s+count-run*4+i*4+3);
	     }
	   }
      } while ((count<srcLen) && (count2<dstLen));
    }
    
    return count2;
}

#endif
