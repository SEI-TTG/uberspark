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

// peh-x86vmx-main.c
// EMHF partition event-hub for Intel x86 vmx
// author: amit vasudevan (amitvasudevan@acm.org)
#include <xmhf-core.h> 
#include <xc-x86.h>
#include <xc-x86vmx.h>

void xmhf_parteventhub_arch_x86vmx_entry(void) __attribute__((naked)){
		//step-1: save all CPU GPRs
		asm volatile ("pushal\r\n");
		
		//step-2: grab VCPU *
		asm volatile ("movl 32(%esp), %esi\r\n");
			      
		//step-3: get hold of pointer to saved GPR on stack
		asm volatile ("movl %esp, %eax\r\n");

		//step-4: invoke "C" event handler
		//1st argument is VCPU * followed by pointer to saved GPRs
		asm volatile ("pushl %eax\r\n");
		asm volatile ("pushl %esi\r\n");
		asm volatile ("call xmhf_parteventhub_arch_x86vmx_intercept_handler\r\n");
		asm volatile ("addl $0x08, %esp\r\n");

		//step-5; restore all CPU GPRs
		asm volatile ("popal\r\n");

		//resume partition
		asm volatile ("vmresume\r\n");
              
		//if we get here then vm resume failed, just bail out with a BP exception 
		asm volatile ("int $0x03\r\n");
		asm volatile ("hlt\r\n");
}

//---VMX decode assist----------------------------------------------------------
//map a CPU register index into appropriate VCPU *vcpu or struct regs *r field 
//and return the address of the field
static u32 * _vmx_decode_reg(u32 gpr, VCPU *vcpu, struct regs *r){
  if ( ((int)gpr >=0) && ((int)gpr <= 7) ){
  
	  switch(gpr){
		case 0: return ( (u32 *)&r->eax );
		case 1: return ( (u32 *)&r->ecx );
		case 2: return ( (u32 *)&r->edx );
		case 3: return ( (u32 *)&r->ebx );
		case 4: return ( (u32 *)&vcpu->vmcs.guest_RSP);
		case 5: return ( (u32 *)&r->ebp );
		case 6: return ( (u32 *)&r->esi );
		case 7: return ( (u32 *)&r->edi );
	  }
   }else{
		printf("\n[%02x]%s: invalid gpr value (%u). halting!", vcpu->id,
			__FUNCTION__, gpr);
		HALT();
   }

	//we will never get here, appease the compiler
	return (u32 *)&r->eax;
}


//---intercept handler (CPUID)--------------------------------------------------
static void _vmx_handle_intercept_cpuid(VCPU *vcpu, struct regs *r){
	//printf("\nCPU(0x%02x): CPUID", vcpu->id);
	asm volatile ("cpuid\r\n"
          :"=a"(r->eax), "=b"(r->ebx), "=c"(r->ecx), "=d"(r->edx)
          :"a"(r->eax), "c" (r->ecx));
	vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;
}






//------------------------------------------------------------------------------
// guest MSR r/w intercept handling
// HAL invokes NT kernel via SYSENTER if CPU supports it. However,
// regular apps using NTDLL will still use INT 2E if registry entry is not
// tweaked. So, we HAVE to emulate SYSENTER_CS/EIP/ESP to ensure that
// NT kernel doesnt panic with SESSION5_INITIALIZATION_FAILED!
//
// This took me nearly a month of disassembly into the HAL, 
// NTKERNEL and debugging to figure out..eh? 
//
// AMD SVM is neater, only
// when you ask for these MSR intercepts do they get stored and read from
// the VMCB. However, for Intel regardless they get stored and read from VMCS
// for the guest. So we need to have these intercepts bare minimum!!
// A line to this effect would have been much appreciated in the Intel manuals
// doh!!!
//------------------------------------------------------------------------------
  
//---intercept handler (WRMSR)--------------------------------------------------
static void _vmx_handle_intercept_wrmsr(VCPU *vcpu, struct regs *r){
	//printf("\nCPU(0x%02x): WRMSR 0x%08x", vcpu->id, r->ecx);

	switch(r->ecx){
		case IA32_SYSENTER_CS_MSR:
			vcpu->vmcs.guest_SYSENTER_CS = (unsigned int)r->eax;
			break;
		case IA32_SYSENTER_EIP_MSR:
			vcpu->vmcs.guest_SYSENTER_EIP = (unsigned long long)r->eax;
			break;
		case IA32_SYSENTER_ESP_MSR:
			vcpu->vmcs.guest_SYSENTER_ESP = (unsigned long long)r->eax;
			break;
		default:{
			asm volatile ("wrmsr\r\n"
          : //no outputs
          :"a"(r->eax), "c" (r->ecx), "d" (r->edx));	
			break;
		}
	}
	
	vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;
	//printf("\nCPU(0x%02x): WRMSR end", vcpu->id);
}

//---intercept handler (RDMSR)--------------------------------------------------
static void _vmx_handle_intercept_rdmsr(VCPU *vcpu, struct regs *r){
	//printf("\nCPU(0x%02x): RDMSR 0x%08x", vcpu->id, r->ecx);

	switch(r->ecx){
		case IA32_SYSENTER_CS_MSR:
			r->eax = (u32)vcpu->vmcs.guest_SYSENTER_CS;
			r->edx = 0;
			break;
		case IA32_SYSENTER_EIP_MSR:
			r->eax = (u32)vcpu->vmcs.guest_SYSENTER_EIP;
			r->edx = 0;
			break;
		case IA32_SYSENTER_ESP_MSR:
			r->eax = (u32)vcpu->vmcs.guest_SYSENTER_ESP;
			r->edx = 0;
			break;
		default:{
			asm volatile ("rdmsr\r\n"
          : "=a"(r->eax), "=d"(r->edx)
          : "c" (r->ecx));
			break;
		}
	}
	vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;

	//printf("\nCPU(0x%02x): RDMSR (0x%08x)=0x%08x%08x", vcpu->id, r->ecx, r->edx, r->eax);
}


//---intercept handler (EPT voilation)----------------------------------
static void _vmx_handle_intercept_eptviolation(context_desc_t context_desc, struct regs *r __attribute__((unused))){
	u32 errorcode, gpa, gva;
	VCPU *vcpu = (VCPU *)&g_bplt_vcpu[context_desc.cpu_desc.id];
	errorcode = (u32)vcpu->vmcs.info_exit_qualification;
	gpa = (u32) vcpu->vmcs.guest_paddr_full;
	gva = (u32) vcpu->vmcs.info_guest_linear_address;

	//check if EPT violation is due to LAPIC interception
	//if(vcpu->isbsp && (gpa >= g_vmx_lapic_base) && (gpa < (g_vmx_lapic_base + PAGE_SIZE_4K)) ){
	//	xmhf_smpguest_arch_x86vmx_eventhandler_hwpgtblviolation(context_desc, gpa, errorcode);
	//}else{ //no, pass it to hypapp 
		xmhf_smpguest_arch_x86vmx_quiesce(vcpu);
		xmhfhypapp_handleintercept_hwpgtblviolation(context_desc, gpa, gva,	(errorcode & 7));
		xmhf_smpguest_arch_x86vmx_endquiesce(vcpu);
	//}		
}


//---intercept handler (I/O port access)----------------------------------------
static void _vmx_handle_intercept_ioportaccess(context_desc_t context_desc, struct regs *r __attribute__((unused))){
  VCPU *vcpu = (VCPU *)&g_bplt_vcpu[context_desc.cpu_desc.id];
  u32 access_size, access_type, portnum, stringio;
	u32 app_ret_status = APP_IOINTERCEPT_CHAIN;
	
  access_size = (u32)vcpu->vmcs.info_exit_qualification & 0x00000007UL;
	access_type = ((u32)vcpu->vmcs.info_exit_qualification & 0x00000008UL) >> 3;
	portnum =  ((u32)vcpu->vmcs.info_exit_qualification & 0xFFFF0000UL) >> 16;
	stringio = ((u32)vcpu->vmcs.info_exit_qualification & 0x00000010UL) >> 4;
	
  HALT_ON_ERRORCOND(!stringio);	//we dont handle string IO intercepts

  //call our app handler, TODO: it should be possible for an app to
  //NOT want a callback by setting up some parameters during appmain
	xmhf_smpguest_arch_x86vmx_quiesce(vcpu);
	app_ret_status=xmhfhypapp_handleintercept_portaccess(context_desc, portnum, access_type, access_size);
    xmhf_smpguest_arch_x86vmx_endquiesce(vcpu);

  if(app_ret_status == APP_IOINTERCEPT_CHAIN){
   	if(access_type == IO_TYPE_OUT){
  		if( access_size== IO_SIZE_BYTE)
  				outb((u8)r->eax, portnum);
  		else if (access_size == IO_SIZE_WORD)
  				outw((u16)r->eax, portnum);
  		else if (access_size == IO_SIZE_DWORD)
  				outl((u32)r->eax, portnum);	
  	}else{
  		if( access_size== IO_SIZE_BYTE){
  				r->eax &= 0xFFFFFF00UL;	//clear lower 8 bits
  				r->eax |= (u8)inb(portnum);
  		}else if (access_size == IO_SIZE_WORD){
  				r->eax &= 0xFFFF0000UL;	//clear lower 16 bits
  				r->eax |= (u16)inw(portnum);
  		}else if (access_size == IO_SIZE_DWORD){
  				r->eax = (u32)inl(portnum);	
  		}
  	}
  	vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;

  }else{
    //skip the IO instruction, app has taken care of it
  	vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;
  }

	return;
}


//---CR0 access handler-------------------------------------------------
static void vmx_handle_intercept_cr0access_ug(VCPU *vcpu, struct regs *r, u32 gpr, u32 tofrom){
	u32 cr0_value;
	
	HALT_ON_ERRORCOND(tofrom == VMX_CRX_ACCESS_TO);
	
	cr0_value = *((u32 *)_vmx_decode_reg(gpr, vcpu, r));

	//printf("\n[cr0-%02x] MOV TO, current=0x%08x, proposed=0x%08x", vcpu->id,
	//	(u32)vcpu->vmcs.guest_CR0, cr0_value);

	vcpu->vmcs.control_CR0_shadow = cr0_value;
	vcpu->vmcs.guest_CR0 = cr0_value & ~(CR0_CD | CR0_NW);
	
	//flush mappings
	xmhf_memprot_arch_x86vmx_flushmappings(vcpu);
}

//---CR4 access handler---------------------------------------------------------
static void vmx_handle_intercept_cr4access_ug(VCPU *vcpu, struct regs *r, u32 gpr, u32 tofrom){
  if(tofrom == VMX_CRX_ACCESS_TO){
	u32 cr4_proposed_value;
	
	cr4_proposed_value = *((u32 *)_vmx_decode_reg(gpr, vcpu, r));
	
	printf("\nCPU(0x%02x): CS:EIP=0x%04x:0x%08x MOV CR4, xx", vcpu->id,
		(u16)vcpu->vmcs.guest_CS_selector, (u32)vcpu->vmcs.guest_RIP);
  
	printf("\nMOV TO CR4 (flush TLB?), current=0x%08x, proposed=0x%08x",
			(u32)vcpu->vmcs.guest_CR4, cr4_proposed_value);

	#if defined (__NESTED_PAGING__)
	//we need to flush EPT mappings as we emulated CR4 load above
	__vmx_invvpid(VMX_INVVPID_SINGLECONTEXT, 1, 0);
	#endif
  }

}

//---XSETBV intercept handler-------------------------------------------
static void _vmx_handle_intercept_xsetbv(VCPU *vcpu, struct regs *r){
	u64 xcr_value;
	
	xcr_value = ((u64)r->edx << 32) + (u64)r->eax;
	
	if(r->ecx != XCR_XFEATURE_ENABLED_MASK){
			printf("\n%s: unhandled XCR register %u", __FUNCTION__, r->ecx);
			HALT();
	}

	//XXX: TODO: check for invalid states and inject GP accordingly
	
	printf("\n%s: xcr_value=%llx", __FUNCTION__, xcr_value);
	
	//XXX: debug: dump CR4 contents
	{
		u32 t_cr4;
		t_cr4 = read_cr4();
		printf("\n%s: host cr4 value=%08x", __FUNCTION__, t_cr4);
	}
	
	//set XCR with supplied value
	xsetbv(XCR_XFEATURE_ENABLED_MASK, xcr_value);

	//skip the emulated XSETBV instruction
	vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;
}						
			


//---hvm_intercept_handler------------------------------------------------------
u32 xmhf_parteventhub_arch_x86vmx_intercept_handler(VCPU *vcpu, struct regs *r){
	context_desc_t context_desc;
	
	context_desc.partition_desc.id = 0;
	context_desc.cpu_desc.id = vcpu->idx;
	context_desc.cpu_desc.isbsp = vcpu->isbsp;

	//read VMCS from physical CPU/core
#ifndef __XMHF_VERIFICATION__
	xmhf_baseplatform_arch_x86vmx_getVMCS(vcpu);
#endif //__XMHF_VERIFICATION__
	//sanity check for VM-entry errors
	if( (u32)vcpu->vmcs.info_vmexit_reason & 0x80000000UL ){
		printf("\nVM-ENTRY error: reason=0x%08x, qualification=0x%016llx", 
			(u32)vcpu->vmcs.info_vmexit_reason, (u64)vcpu->vmcs.info_exit_qualification);
		xmhf_baseplatform_arch_x86vmx_dumpVMCS(vcpu);
		HALT();
	}

	/*{
		u32 eax, edx;
		rdmsr(MSR_EFER, &eax, &edx);
		printf("\n%s: MSR_EFER=%08x%08x", __FUNCTION__, edx, eax);
	}*/

  
	//handle intercepts
	switch((u32)vcpu->vmcs.info_vmexit_reason){
		//--------------------------------------------------------------
		//xmhf-core and hypapp intercepts
		//--------------------------------------------------------------
		
		case VMX_VMEXIT_VMCALL:{
			//if INT 15h E820 hypercall, then let the xmhf-core handle it
			if(vcpu->vmcs.guest_CS_base == (VMX_UG_E820HOOK_CS << 4) &&
				vcpu->vmcs.guest_RIP == VMX_UG_E820HOOK_IP){
				//we need to be either in real-mode or in protected
				//mode with paging and EFLAGS.VM bit set (virtual-8086 mode)
				HALT_ON_ERRORCOND( !(vcpu->vmcs.guest_CR0 & CR0_PE)  ||
					( (vcpu->vmcs.guest_CR0 & CR0_PE) && (vcpu->vmcs.guest_CR0 & CR0_PG) &&
						(vcpu->vmcs.guest_RFLAGS & EFLAGS_VM)  ) );
				//_vmx_int15_handleintercept(vcpu, r);	
				xmhf_smpguest_arch_x86vmx_handle_guestmemoryreporting(context_desc, r);

			}else{	//if not E820 hook, give hypapp a chance to handle the hypercall
				xmhf_smpguest_arch_x86vmx_quiesce(vcpu);
				{
					u64 hypercall_id = (u64)r->eax;
					u64 hypercall_param = ((u64)r->edx << 32) | r->ecx;
					if( xmhfhypapp_handlehypercall(context_desc, hypercall_id, hypercall_param) != APP_SUCCESS){
						printf("\nCPU(0x%02x): error(halt), unhandled hypercall 0x%08x!", vcpu->id, r->eax);
						HALT();
					}
				}
				xmhf_smpguest_arch_x86vmx_endquiesce(vcpu);
				vcpu->vmcs.guest_RIP += 3;
			}
		}
		break;

		case VMX_VMEXIT_IOIO:{
			_vmx_handle_intercept_ioportaccess(context_desc, r);
		}
		break;

		case VMX_VMEXIT_EPT_VIOLATION:{
			_vmx_handle_intercept_eptviolation(context_desc, r);
		}
		break;  

		case VMX_VMEXIT_INIT:{
			printf("\n***** VMEXIT_INIT xmhfhypapp_handleshutdown\n");
			xmhfhypapp_handleshutdown(context_desc);      
			printf("\nCPU(0x%02x): Fatal, xmhfhypapp_handleshutdown returned. Halting!", vcpu->id);
			HALT();
		}
		break;

		//--------------------------------------------------------------
		//xmhf-core only intercepts
		//--------------------------------------------------------------
		case VMX_VMEXIT_HLT:
			if(!vcpu->vmx_guest_unrestricted){
				printf("\nCPU(0x%02x): V86 monitor based real-mode exec. unsupported!", vcpu->id);
				HALT();
			}else{
				printf("\nCPU(0x%02x): Unexpected HLT intercept in UG, Halting!", vcpu->id);
				HALT();
			}
		break;

 		case VMX_VMEXIT_EXCEPTION:{
			switch( ((u32)vcpu->vmcs.info_vmexit_interrupt_information & INTR_INFO_VECTOR_MASK) ){
				//case 0x01:
				//	xmhf_smpguest_arch_x86vmx_eventhandler_dbexception(context_desc, r);
				//	break;				
				
				case 0x02:	//NMI
					#ifndef __XMHF_VERIFICATION__
					//we currently discharge quiescing via manual inspection
					xmhf_smpguest_arch_x86vmx_eventhandler_nmiexception(vcpu, r);
					#endif // __XMHF_VERIFICATION__
					break;
				
				default:
					printf("\nVMEXIT-EXCEPTION:");
					printf("\ncontrol_exception_bitmap=0x%08lx",
						(unsigned long)vcpu->vmcs.control_exception_bitmap);
					printf("\ninterruption information=0x%08lx", 
						(unsigned long)vcpu->vmcs.info_vmexit_interrupt_information);
					printf("\nerrorcode=0x%08lx", 
						(unsigned long)vcpu->vmcs.info_vmexit_interrupt_error_code);
					HALT();
			}
		}
		break;

 		case VMX_VMEXIT_CRX_ACCESS:{
			u32 tofrom, gpr, crx; 
			//printf("\nVMEXIT_CRX_ACCESS:");
			//printf("\ninstruction length: %u", info_vmexit_instruction_length);
			crx=(u32) ((u64)vcpu->vmcs.info_exit_qualification & 0x000000000000000FULL);
			gpr=
			 (u32) (((u64)vcpu->vmcs.info_exit_qualification & 0x0000000000000F00ULL) >> (u64)8);
			tofrom = 
			(u32) (((u64)vcpu->vmcs.info_exit_qualification & 0x0000000000000030ULL) >> (u64)4); 
			//printf("\ncrx=%u, gpr=%u, tofrom=%u", crx, gpr, tofrom);

			if ( ((int)gpr >=0) && ((int)gpr <= 7) ){
				switch(crx){
					case 0x0: //CR0 access
						vmx_handle_intercept_cr0access_ug(vcpu, r, gpr, tofrom);	
						break;
					
					case 0x4: //CR4 access
						if(!vcpu->vmx_guest_unrestricted){
							printf("\nHALT: v86 monitor based real-mode exec. unsupported!");
							HALT();
						}else{
							vmx_handle_intercept_cr4access_ug(vcpu, r, gpr, tofrom);	
						}
						break;
				
					default:
						printf("\nunhandled crx, halting!");
						HALT();
				}
				vcpu->vmcs.guest_RIP += vcpu->vmcs.info_vmexit_instruction_length;
			}else{
				printf("\n[%02x]%s: invalid gpr value (%u). halting!", vcpu->id,
					__FUNCTION__, gpr);
				HALT();
			}
		}
		break;	

 		case VMX_VMEXIT_RDMSR:
			_vmx_handle_intercept_rdmsr(vcpu, r);
			break;
			
		case VMX_VMEXIT_WRMSR:
			_vmx_handle_intercept_wrmsr(vcpu, r);
			break;
			
		case VMX_VMEXIT_CPUID:
			_vmx_handle_intercept_cpuid(vcpu, r);
			break;

		case VMX_VMEXIT_TASKSWITCH:{
			u32 idt_v = vcpu->vmcs.info_IDT_vectoring_information & VECTORING_INFO_VALID_MASK;
			u32 type = vcpu->vmcs.info_IDT_vectoring_information & VECTORING_INFO_TYPE_MASK;
			u32 reason = vcpu->vmcs.info_exit_qualification >> 30;
			u16 tss_selector = (u16)vcpu->vmcs.info_exit_qualification;
			
			if(reason == TASK_SWITCH_GATE && type == INTR_TYPE_NMI){
				printf("\nCPU(0x%02x): NMI received (MP guest shutdown?)", vcpu->id);
					xmhfhypapp_handleshutdown(context_desc);      
				printf("\nCPU(0x%02x): warning, xmhfhypapp_handleshutdown returned!", vcpu->id);
				printf("\nCPU(0x%02x): HALTING!", vcpu->id);
				HALT();
			}else{
				printf("\nCPU(0x%02x): Unhandled Task Switch. Halt!", vcpu->id);
				printf("\n	idt_v=0x%08x, type=0x%08x, reason=0x%08x, tsssel=0x%04x",
					idt_v, type, reason, tss_selector); 
			}
			HALT();
		}
		break;

		case VMX_VMEXIT_XSETBV:{
			_vmx_handle_intercept_xsetbv(vcpu, r);
		}
		break;

		case VMX_VMEXIT_SIPI:{
			u32 sipivector = (u8)vcpu->vmcs.info_exit_qualification;
			printf("\nCPU(%02x): SIPI vector=0x%08x, Halting!", vcpu->id, sipivector);
			vcpu->vmcs.guest_CS_selector = ((sipivector * PAGE_SIZE_4K) >> 4); 
			vcpu->vmcs.guest_CS_base = (sipivector * PAGE_SIZE_4K); 
			vcpu->vmcs.guest_RIP = 0x0ULL;
			vcpu->vmcs.guest_activity_state=0;	//active
		}
		break;

    
		default:{
			printf("\nCPU(0x%02x): Unhandled intercept: 0x%08x", vcpu->id, (u32)vcpu->vmcs.info_vmexit_reason);
			printf("\n	CPU(0x%02x): EFLAGS=0x%08x", vcpu->id, (u32)vcpu->vmcs.guest_RFLAGS);
			printf("\n	SS:ESP =0x%04x:0x%08x", (u16)vcpu->vmcs.guest_SS_selector, (u32)vcpu->vmcs.guest_RSP);
			printf("\n	CS:EIP =0x%04x:0x%08x", (u16)vcpu->vmcs.guest_CS_selector, (u32)vcpu->vmcs.guest_RIP);
			printf("\n	IDTR base:limit=0x%08x:0x%04x", (u32)vcpu->vmcs.guest_IDTR_base,
					(u16)vcpu->vmcs.guest_IDTR_limit);
			printf("\n	GDTR base:limit=0x%08x:0x%04x", (u32)vcpu->vmcs.guest_GDTR_base,
					(u16)vcpu->vmcs.guest_GDTR_limit);
			if(vcpu->vmcs.info_IDT_vectoring_information & 0x80000000){
				printf("\nCPU(0x%02x): HALT; Nested events unhandled 0x%08x",
					vcpu->id, vcpu->vmcs.info_IDT_vectoring_information);
			}
			HALT();
		}		
	} //end switch((u32)vcpu->vmcs.info_vmexit_reason)
	

 	//check and clear guest interruptibility state
	if(vcpu->vmcs.guest_interruptibility != 0){
		vcpu->vmcs.guest_interruptibility = 0;
	}

	//make sure we have no nested events
	if(vcpu->vmcs.info_IDT_vectoring_information & 0x80000000){
		printf("\nCPU(0x%02x): HALT; Nested events unhandled with hwp:0x%08x",
			vcpu->id, vcpu->vmcs.info_IDT_vectoring_information);
		HALT();
	}

	//write updated VMCS back to CPU
#ifndef __XMHF_VERIFICATION__
	xmhf_baseplatform_arch_x86vmx_putVMCS(vcpu);
#endif // __XMHF_VERIFICATION__


#ifdef __XMHF_VERIFICATION_DRIVEASSERTS__
	//ensure that whenever a partition is resumed on a vcpu, we have extended paging
	//enabled and that the base points to the extended page tables we have initialized
	assert( (vcpu->vmcs.control_VMX_seccpu_based & 0x2) );
	assert( (vcpu->vmcs.control_EPT_pointer_high == 0) && (vcpu->vmcs.control_EPT_pointer_full == (hva2spa((void*)vcpu->vmx_vaddr_ept_pml4_table) | 0x1E)) );
#endif	

	return 1;
}