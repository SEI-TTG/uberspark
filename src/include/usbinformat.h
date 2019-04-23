/*
 * @XMHF_LICENSE_HEADER_START@
 *
 * eXtensible, Modular Hypervisor Framework (XMHF)
 * Copyright (c) 2009-2012 Carnegie Mellon University
 * Copyright (c) 2010-2012 VDG Inc.
 * All Rights Reserved.
 *
 * Developed by: XMHF Team
 *               Carnegie Mellon University / CyLab
 *               VDG Inc.
 *               http://xmhf.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of Carnegie Mellon or VDG Inc, nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @XMHF_LICENSE_HEADER_END@
 */

// uberSpark binary format definitions
//author: amit vasudevan (amitvasudevan@acm.org)

#ifndef __UBERSPARK_BINFORMAT_H__
#define __UBERSPARK_BINFORMAT_H__

#define USBINFORMAT_SECTION_TYPE_UOBJ							0x1
#define USBINFORMAT_SECTION_TYPE_UOBJCOLL_ENTRYSENTINEL			0x2
#define USBINFORMAT_SECTION_TYPE_UOBJ_RESUMESENTINEL			0x3
#define USBINFORMAT_SECTION_TYPE_UOBJ_CALLEESENTINEL			0x4
#define USBINFORMAT_SECTION_TYPE_UOBJ_EXITCALLEESENTINEL		0x5
#define USBINFORMAT_SECTION_TYPE_UOBJ_CODE						0x6
#define USBINFORMAT_SECTION_TYPE_UOBJ_RWDATA					0x7
#define USBINFORMAT_SECTION_TYPE_UOBJ_RODATA					0x8
#define USBINFORMAT_SECTION_TYPE_UOBJ_USTACK					0x9
#define USBINFORMAT_SECTION_TYPE_UOBJ_TSTACK					0xa
#define USBINFORMAT_SECTION_TYPE_UOBJ_USTACKTOS					0xb
#define USBINFORMAT_SECTION_TYPE_UOBJ_TSTACKTOS					0xc


#ifndef __ASSEMBLY__

typedef struct {
	uint32_t type;			//section type
	uint32_t prot;			//section protections
	uint32_t va_offset;		//virtual address offset from load address
	uint32_t file_offset;	//offset within the binary file
	uint32_t size;			//size of the section in bytes
	uint32_t aligned_at;	//boundary that section is aligned at
	uint32_t pad_to;		//boundary that section is padded to
	uint32_t reserved;		//reserved
} __attribute__((packed)) usbinformat_section_info_t;




#endif //__ASSEMBLY__


#endif //__UBERSPARK_BINFORMAT_H__
