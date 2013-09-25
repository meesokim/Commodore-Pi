#include "C64.h"
#include "6510.h"
#include "C64Data.h"
//#include "c64func.h"
//#include "c64proc.h"
//#include "sound.h"

//#define UNDOCd


inline void CB64_6510()
{
	static CB64_pair K;
	static WORD address, jmp_from;
	static BYTE gen1;
	static BYTE curr_inst = 0x80;
	static curr_loc = 0xFCE2;
	static BYTE curr_cycle;
	static BYTE a_reg=0,x_reg=0,y_reg=0,flags=0x30,stack_ptr = 0xFF;

	// 6510
	switch(curr_inst) {
	case 0x80:
		if(done)
			return;
		if(reset==1)
		{
			curr_loc= (WORD) (KernalPointer[0x1FFD]<<8) + (WORD) (KernalPointer[0x1FFC]);
			stack_ptr=0xFF;
			flags=0x30;
			CB64_ClearMem();
			IRQ=NMI=reset=0;
		}
		else if(reset==2)
		{
			RAMPointer[1]=55;
			curr_inst=0x80;
			curr_loc=2456;
			IRQ=NMI=reset=0;
		}

		if(IRQ)
		{
			if(flags&FLAG_INT)goto NOIRQ;
			Poke(0x100+stack_ptr--,curr_loc>>8);
			Poke(0x100+stack_ptr--,curr_loc&255);
			Poke(0x100+stack_ptr--,flags&BRK_OFF);
			curr_loc=Deek(0xFFFE);
			flags|=FLAG_INT;
		}
/*on reset sound off		if (curr_loc==0xfce2) {SNDkeys&=~(1<<0);SNDkeys&=~(1<<1);SNDkeys&=~(1<<2);}*/
		/*load program*/
	
		if ( curr_loc==0xf4b8 && Peek(186) == 8 ) 
		{
//			curr_loc=Basic2_ProgLoad();
			if (curr_loc==0xf5d2) 
				flags &= ~FLAG_CARRY;
		}
		

		if(NMI)
		{
			NMI=0;
			Poke(0x100+stack_ptr--,(curr_loc)>>8);
			Poke(0x100+stack_ptr--,(curr_loc)&255);
			Poke(0x100+stack_ptr--,flags);
			curr_loc=Deek(0xFFFA);
		}
NOIRQ:                  
		if((curr_inst=Peek(curr_loc))==0x80)
			curr_inst=0x82;
		curr_cycle=1;
		break;
	case 0xA5: //LDA_Zero();
		if(curr_cycle==1)
		{
			Zero();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(curr_cycle++==2)
			curr_inst=0x80;
		break;
	case 0xD0: //BNE();
		if(curr_cycle==1)curr_loc+=2;
		if((flags&FLAG_ZERO)==0)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else {curr_inst=0x80;}
		break;
	case 0x4C: //JMP_Abs();
		if(curr_cycle==1)
		{
			jmp_from=curr_loc;
			curr_loc++;
			curr_loc=Deek(curr_loc);
		}
		if(curr_cycle++==2)	curr_inst=0x80;
		break;
	case 0xE8: //INX();
		x_reg++;
		setflags(x_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x10: //BPL();
		if(curr_cycle==1)curr_loc+=2;
		if((flags&FLAG_NEGATIVE)==0)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)
				curr_inst=0x80;
		}
		else
			curr_inst=0x80;
		break;
	case 0xC9: //CMP_Imm();
		Imm();
		CMP();
		curr_inst=0x80;
		break;

		
	case 0x00: //BRK();
		if(curr_cycle==1)
		{
			Poke(0x100+stack_ptr--,(curr_loc+2)>>8);
			Poke(0x100+stack_ptr--,(curr_loc+2)&255);
			Poke(0x100+stack_ptr--,flags);
			flags|=FLAG_INT;
			curr_loc=Deek(0xFFFE);
		}
		if(curr_cycle++==6)	curr_inst=0x80;
		break;
	case 0x01: //ORA_IndX();
		if(curr_cycle==1)
		{
			IndX();
			ORA();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x05: //ORA_Zero();
		if(curr_cycle==1)
		{
			Zero();
			ORA();
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0x06: //ASL_Zero();
		if(curr_cycle==1)
		{
			Zero();
			ASL();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x08: //PHP();
		if(curr_cycle==1)
		{
			Poke(0x100+stack_ptr--,flags);
			curr_loc+=1;
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0x09: //ORA_Imm();
		Imm();
		ORA();
		curr_inst=0x80;
		break;
	case 0x0A: //ASL_Acc();
		if(a_reg&0x80)flags|=FLAG_CARRY;else flags&=(CRRY_OFF);
		a_reg<<=1;
		setflags(a_reg);
		curr_inst=0x80;
		curr_loc++;
		break;
	case 0x0D: //ORA_Abs();
		if(curr_cycle==1)
		{
			Abs();
			ORA();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x0E: //ASL_Abs();
		if(curr_cycle==1)
		{
			Abs();
			ASL();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;

	case 0x11: //ORA_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			ORA();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x15: //ORA_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			ORA();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x16: //ASL_Zero_X();
		if(curr_cycle++==1)
		{
			Zero_X();
			ASL();
		}
		if(curr_cycle==5)curr_inst=0x80;
		break;
	case 0x18: //CLC();
		flags&=(CRRY_OFF);
		curr_inst=0x80;
		curr_loc++;
		break;
	case 0x19: //ORA_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			ORA();
		}
		if(((address-y_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x1D: //ORA_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			ORA();
		}
		if(((address-x_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else 
			if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x1E: //ASL_Abs_X();
		if(curr_cycle++==1)
		{
			Abs_X();
			ASL();
		}
		if(curr_cycle==6)curr_inst=0x80;
		break;
	case 0x20: //JSR();
		curr_loc+=2;
		jmp_from=curr_loc;
		Poke(0x100+stack_ptr--,curr_loc>>8);
		Poke(0x100+stack_ptr--,curr_loc&255);
		curr_loc=Deek(curr_loc-1);
		curr_inst=0x80;
		break;
	case 0x21: //AND_IndX();
		if(curr_cycle==1)
		{
			IndX();
			AND();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x24: //BIT_Zero();
		if(curr_cycle==1)
		{
			Zero();
			BIT();
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0x25: //AND_Zero();
		if(curr_cycle==1)
		{
			Zero();
			AND();
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0x26: //ROL_Zero();
		if(curr_cycle==1)
		{
			Zero();
			ROL();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x28: //PLP();
		if(curr_cycle==1)
		{
			flags=Peek(0x100+(++stack_ptr))|0x30;
		}
		if(curr_cycle++==3)
		{
			curr_inst=0x80;curr_loc++;
		}
		break;
	case 0x29: //AND_Imm();
		Imm();
		AND();
		curr_inst=0x80;
		break;
	case 0x2A: //ROL_Acc();
		gen1=a_reg<<1;
		if(flags&FLAG_CARRY)gen1|=1;
		if(a_reg&0x80)
			flags|=FLAG_CARRY;
		else
			flags&=CRRY_OFF;
		setflags(gen1);
		a_reg=gen1;
		curr_inst=0x80;curr_loc++;
		break;
	case 0x2C: //BIT_Abs();
		if(curr_cycle==1)
		{
			Abs();
			BIT();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x2D: //AND_Abs();
		if(curr_cycle==1)
		{
			Abs();
			AND();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x2E: //ROL_Abs();
		if(curr_cycle==1)
		{
			Abs();
			ROL();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x30: //BMI();
		if(curr_cycle==1)curr_loc+=2;
		if(flags&FLAG_NEGATIVE)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else{curr_inst=0x80;}
		break;
	case 0x31: //AND_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			AND();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x35: //AND_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			AND();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x36: //ROL_Zero_X();
		if(curr_cycle++==1)
		{
			Zero_X();
			ROL();
		}
		if(curr_cycle==5)curr_inst=0x80;
		break;
	case 0x38: //SEC();
		flags|=FLAG_CARRY;
		curr_inst=0x80;
		curr_loc++;
		break;
	case 0x39: //AND_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			AND();
		}
		if(((address-y_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x3D: //AND_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			AND();
		}
		if(((address-x_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x3E: //ROL_Abs_X();
		if(curr_cycle++==1)
		{
			Abs_X();
			ROL();
		}
		if(curr_cycle==6)curr_inst=0x80;
		break;
	case 0x40: //RTI();
		if(curr_cycle==1)
		{
			flags=Peek(0x100+(++stack_ptr))|0x30;
			K.B.l=Peek(0x100+(++stack_ptr));
			K.B.h=Peek(0x100+(++stack_ptr));
			curr_loc=K.W;
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0x41: //EOR_IndX();
		if(curr_cycle==1)
		{
			IndX();
			EOR();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x45: //EOR_Zero();
		if(curr_cycle==1)
		{
			Zero();
			EOR();
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0x46: //LSR_Zero();
		if(curr_cycle==1)
		{
			Zero();
			LSR();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x48: //PHA();
		if(curr_cycle==1)
		{
			Poke(0x100+stack_ptr--,a_reg);
			curr_loc++;
		}
		if(curr_cycle++==2)
		{
			curr_inst=0x80;
		}
		break;
	case 0x49: //EOR_Imm();
		Imm();
		EOR();
		curr_inst=0x80;
		break;
	case 0x4A: //LSR_Acc();
		if(a_reg&1)flags|=FLAG_CARRY;else flags&=(CRRY_OFF);
		a_reg>>=1;
		setflags(a_reg);
		curr_inst=0x80;curr_loc++;
		break;
	case 0x4D: //EOR_Abs();
		if(curr_cycle==1)
		{
			Abs();
			EOR();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x4E: //LSR_Abs();
		if(curr_cycle==1)
		{
			Abs();
			LSR();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x50: //BVC();
		if(curr_cycle==1)curr_loc+=2;
		if((flags&FLAG_OVERFLOW)==0)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else {curr_inst=0x80;}
		break;
	case 0x51: //EOR_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			EOR();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x55: //EOR_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			EOR();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x56: //LSR_Zero_X();
		if(curr_cycle++==1)
		{
			Zero_X();
			LSR();
		}
		if(curr_cycle==5)curr_inst=0x80;
		break;
	case 0x58: //CLI();
		flags&=(INT_OFF);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x59: //EOR_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			EOR();
		}
		if(((address-y_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x5D: //EOR_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			EOR();
		}
		if(((address-x_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x5E: //LSR_Abs_X();
		if(curr_cycle++==1)
		{
			Abs_X();
			LSR();
		}
		if(curr_cycle==6)curr_inst=0x80;
		break;
	case 0x60: //RTS();
		if(curr_cycle==1)
		{
			K.B.l=Peek(0x100+(++stack_ptr));
			K.B.h=Peek(0x100+(++stack_ptr));
			curr_loc=K.W+1;
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0x61: //ADC_IndX();
		if(curr_cycle==1)
		{
			IndX();
			ADC();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x65: //ADC_Zero();
		if(curr_cycle==1)
		{
			Zero();
			ADC();
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0x66: //ROR_Zero();
		if(curr_cycle==1)
		{
			Zero();
			ROR();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x68: //PLA();
		if(curr_cycle==1)
		{
			a_reg=Peek(0x100+(++stack_ptr));
			setflags(a_reg);
		}
		if(curr_cycle++==3)
		{
			curr_inst=0x80;curr_loc+=1;
		}
		break;
	case 0x69: //ADC_Imm();
		Imm();
		ADC();
		curr_inst=0x80;
		break;
	case 0x6A: //ROR_Acc();
		gen1=a_reg>>1;
		if(flags&FLAG_CARRY)gen1|=0x80;
		if(a_reg&1)
			flags|=FLAG_CARRY;
		else
			flags&=CRRY_OFF;
		setflags(gen1);
		a_reg=gen1;
		curr_inst=0x80;curr_loc++;
		break;
	case 0x6C: //JMP_Ind();
		if(curr_cycle==1)
		{
			jmp_from=curr_loc;
			address=Deek(curr_loc+1);
			K.W=address;
			curr_loc=Peek(K.W);
			K.B.l++;
			curr_loc+=Peek(K.W)<<8;
		}
		if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0x6D: //ADC_Abs();
		if(curr_cycle==1)
		{
			Abs();
			ADC();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x6E: //ROR_Abs();
		if(curr_cycle==1)
		{
			Abs();
			ROR();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x70: //BVS();
		if(curr_cycle==1)curr_loc+=2;
		if(flags&FLAG_OVERFLOW)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else {curr_inst=0x80;}
		break;
	case 0x71: //ADC_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			ADC();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0x75: //ADC_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			ADC();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0x76: //ROR_Zero_X();
		if(curr_cycle++==1)
		{
			Zero_X();
			ROR();
		}
		if(curr_cycle==5)curr_inst=0x80;
		break;
	case 0x78: //SEI();
		flags|=FLAG_INT;
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x79: //ADC_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			ADC();
		}
		if(((address-y_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x7D: //ADC_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			ADC();
		}
		if(((address-x_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0x7E: //ROR_Abs_X();
		if(curr_cycle++==1)
		{
			Abs_X();
			ROR();
		}
		if(curr_cycle==6)curr_inst=0x80;
		break;
	case 0x81: //STA_IndX();
		if(curr_cycle==1)
		{
			IndX();
			Poke(address,a_reg);
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0x84: //STY_Zero();
		if(curr_cycle==1)
		{
			Zero();
			Poke(address,y_reg);
		}
		if(curr_cycle++==2) curr_inst=0x80;
		break;
	case 0x85: //STA_Zero();
		if(curr_cycle==1)
		{
			Zero();
			Poke(address,a_reg);
		}
		if(curr_cycle++==2) curr_inst=0x80;
		break;
	case 0x86: //STX_Zero();
		if(curr_cycle==1)
		{
			Zero();
			Poke(address,x_reg);
		}
		if(curr_cycle++==2) curr_inst=0x80;
		break;
	case 0x88: //DEY();
		y_reg--;
		setflags(y_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x8A: //TXA();
		a_reg=x_reg;
		setflags(a_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x8C: //STY_Abs();
		if(curr_cycle==1)
		{
			Abs();
			Poke(address,y_reg);
		}
		if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0x8D: //STA_Abs();
		if(curr_cycle==1)
		{
			Abs();
			Poke(address,a_reg);
		}
		if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0x8E: //STX_Abs();
		if(curr_cycle==1)
		{
			Abs();
			Poke(address,x_reg);
		}
		if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0x90: //BCC();
		if(curr_cycle==1)curr_loc+=2;
		if((flags&FLAG_CARRY)==0)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else {curr_inst=0x80;}
		break;
	case 0x91: //STA_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			Poke(address,a_reg);
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0x94: //STY_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			Poke(address,y_reg);
		}
		if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0x95: //STA_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			Poke(address,a_reg);
		}
		if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0x96: //STX_Zero_Y();
		if(curr_cycle==1)
		{
			Zero_Y();
			Poke(address,x_reg);
		}
		if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0x98: //TYA();
		a_reg=y_reg;
		setflags(a_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x99: //STA_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			Poke(address,a_reg);
		}
		if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0x9A: //TXS();
		stack_ptr=x_reg;
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0x9D: //STA_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			Poke(address,a_reg);
		}
		if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xA0: //LDY_Imm();
		Imm();
		y_reg=Peek(address);
		setflags(y_reg);
		curr_inst=0x80;
		break;
	case 0xA1: //LDA_IndX();
		if(curr_cycle==1)
		{
			IndX();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(curr_cycle++==5)
			curr_inst=0x80;
		break;
	case 0xA2: //LDX_Imm();
		Imm();
		x_reg=Peek(address);
		setflags(x_reg);
		curr_inst=0x80;
		break;
	case 0xA4: //LDY_Zero();
		if(curr_cycle==1)
		{
			Zero();
			y_reg=Peek(address);
			setflags(y_reg);
		}
		if(curr_cycle++==2)
			curr_inst=0x80;
		break;

	case 0xA6: //LDX_Zero();
		if(curr_cycle==1)
		{
			Zero();
			x_reg=Peek(address);
			setflags(x_reg);
		}
		if(curr_cycle++==2)
			curr_inst=0x80;
		break;
	case 0xA8: //TAY();
		y_reg=a_reg;
		setflags(y_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0xA9: //LDA_Imm();
		Imm();
		a_reg=Peek(address);
		setflags(a_reg);
		curr_inst=0x80;
		break;
	case 0xAA: //TAX();
		x_reg=a_reg;
		setflags(x_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0xAC: //LDY_Abs();
		if(curr_cycle==1)
		{
			Abs();
			y_reg=Peek(address);
			setflags(y_reg);
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xAD: //LDA_Abs();
		if(curr_cycle==1)
		{
			Abs();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xAE: //LDX_Abs();
		if(curr_cycle==1)
		{
			Abs();
			x_reg=Peek(address);
			setflags(x_reg);
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xB0: //BCS();
		if(curr_cycle==1)curr_loc+=2;
		if(flags&FLAG_CARRY)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
				
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else {curr_inst=0x80;}
		break;
	case 0xB1: //LDA_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(((address-y_reg)/256)!=(address/256))
		{
			if(curr_cycle++==5) curr_inst=0x80;
		}
		else if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xB4: //LDY_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			y_reg=Peek(address);
			setflags(y_reg);
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xB5: //LDA_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xB6: //LDX_Zero_Y();
		if(curr_cycle==1)
		{
			Zero_Y();
			x_reg=Peek(address);
			setflags(x_reg);
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xB8: //CLV();
		flags&=(OVER_OFF);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0xB9: //LDA_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(((address-y_reg)/256)!=(address/256))
		{
			if(curr_cycle++==5) curr_inst=0x80;
		}
		else if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xBA: //TSX();
		x_reg=stack_ptr;
		setflags(x_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0xBC: //LDY_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			y_reg=Peek(address);
			setflags(y_reg);
		}
		if(((address-x_reg)/256)!=(address/256))
		{
			if(curr_cycle++==5) curr_inst=0x80;
		}
		else if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xBD: //LDA_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			a_reg=Peek(address);
			setflags(a_reg);
		}
		if(((address-x_reg)/256)!=(address/256))
		{
			if(curr_cycle++==5) curr_inst=0x80;
		}
		else if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xBE: //LDX_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			x_reg=Peek(address);
			setflags(x_reg);
		}
		if(((address-y_reg)/256)!=(address/256))
		{
			if(curr_cycle++==5) curr_inst=0x80;
		}
		else if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xC0: //CPY_Imm();
		Imm();
		CPY();
		curr_inst=0x80;
		break;
	case 0xC1: //CMP_IndX();
		if(curr_cycle==1)
		{
			IndX();
			CMP();
		}
		if(curr_cycle++==5)
			curr_inst=0x80;
		break;
	case 0xC4: //CPY_Zero();
		if(curr_cycle==1)
		{
			Zero();
			CPY();
		}
		if(curr_cycle++==2)
			curr_inst=0x80;
		break;
	case 0xC5: //CMP_Zero();
		if(curr_cycle==1)
		{
			Zero();
			CMP();
		}
		if(curr_cycle++==2)
			curr_inst=0x80;
		break;
	case 0xC6: //DEC_Zero();
		if(curr_cycle==1)
		{
			Zero();
			Poke(address,gen1=Peek(address)-1);
			setflags(gen1);
		}
		if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xC8: //INY();
		y_reg++;
		setflags(y_reg);
		curr_loc++;
		curr_inst=0x80;
		break;

	case 0xCA: //DEX();
		x_reg--;
		setflags(x_reg);
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0xCC: //CPY_Abs();
		if(curr_cycle==1)
		{
			Abs();
			CPY();
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xCD: //CMP_Abs();
		if(curr_cycle==1)
		{
			Abs();
			CMP();
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xCE: //DEC_Abs();
		if(curr_cycle==1)
		{
			Abs();
			Poke(address,gen1=Peek(address)-1);
			setflags(gen1);
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;

	case 0xD1: //CMP_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			CMP();
		}
		if(((address-y_reg)/256)!=(address/256))
		{
			if(curr_cycle++==5) curr_inst=0x80;
		}
		else if(curr_cycle++==4) curr_inst=0x80;
		break;
	case 0xD5: //CMP_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			CMP();
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xD6: //DEC_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			Poke(address,gen1=Peek(address)-1);
			setflags(gen1);
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0xD8: //CLD();
		flags&=(DEC_OFF);
		curr_inst=0x80;
		curr_loc++;
		break;
	case 0xD9: //CMP_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			CMP();
		}
		if(((address-y_reg)/256)!=(address/256))
		{
			if(curr_cycle++==4) curr_inst=0x80;
		}
		else if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0xDD: //CMP_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			CMP();
		}
		if(((address-x_reg)/256)!=(address/256))
		{
			if(curr_cycle++==4) curr_inst=0x80;
		}
		else if(curr_cycle++==3) curr_inst=0x80;
		break;
	case 0xDE: //DEC_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			Poke(address,gen1=Peek(address)-1);
			setflags(gen1);
		}
		if(curr_cycle++==6) curr_inst=0x80;
		break;
		
	case 0xE0: //CPX_Imm();
		Imm();
		CPX();
		curr_inst=0x80;
		break;
	case 0xE1: //SBC_IndX();
		if(curr_cycle==1)
		{
			IndX();
			SBC();
		}
		if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0xE4: //CPX_Zero();
		if(curr_cycle==1)
		{
			Zero();
			CPX();
		}
		if(curr_cycle++==2)
			curr_inst=0x80;
		break;
	case 0xE5: //SBC_Zero();
		if(curr_cycle==1)
		{
			Zero();
			SBC();
		}
		if(curr_cycle++==2)curr_inst=0x80;
		break;
	case 0xE6: //INC_Zero();
		if(curr_cycle==1)
		{
			Zero();
			Poke(address,gen1=Peek(address)+1);
			setflags(gen1);
		}
		if(curr_cycle++==4) curr_inst=0x80;
		break;

	case 0xE9: //SBC_Imm();
		Imm();
		SBC();
		curr_inst=0x80;
		break;
		
		// UNDOC'D
	case 0x1A:
	case 0x3A:
	case 0x5A:
	case 0x7A:
	case 0xDA:
	case 0xFA:
	case 0xEA: //NOP();
		curr_loc++;
		curr_inst=0x80;
		break;
	case 0xEC: //CPX_Abs();
		if(curr_cycle==1)
		{
			Abs();
			CPX();
		}
		if(curr_cycle++==3)
			curr_inst=0x80;
		break;
	case 0xED: //SBC_Abs();
		if(curr_cycle==1)
		{
			Abs();
			SBC();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0xEE: //INC_Abs();
		if(curr_cycle==1)
		{
			Abs();
			Poke(address,gen1=Peek(address)+1);
			setflags(gen1);
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0xF0: //BEQ();
		if(curr_cycle==1)curr_loc+=2;
		if(flags&FLAG_ZERO)
		{
			if(curr_cycle==2)
			{
				
				curr_loc+=(signed char)(gen1=Peek(curr_loc-1));
				if((curr_loc/256)==((curr_loc+(signed char)gen1)/256));
				{
					curr_inst=0x80;
				}
			}
			if(curr_cycle++==3)curr_inst=0x80;
		}
		else {curr_inst=0x80;}
		break;
	case 0xF1: //SBC_Ind_Y();
		if(curr_cycle==1)
		{
			Ind_Y();
			SBC();
		}
		if(curr_cycle++==4)curr_inst=0x80;
		break;
	case 0xF5: //SBC_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			SBC();
		}
		if(curr_cycle++==3)curr_inst=0x80;
		break;
	case 0xF6: //INC_Zero_X();
		if(curr_cycle==1)
		{
			Zero_X();
			Poke(address,gen1=Peek(address)+1);
			setflags(gen1);
		}
		if(curr_cycle++==5) curr_inst=0x80;
		break;
	case 0xF8: //SED();
		flags|=FLAG_DECIMAL;
		curr_inst=0x80;
		curr_loc++;
		break;
	case 0xF9: //SBC_Abs_Y();
		if(curr_cycle==1)
		{
			Abs_Y();
			SBC();
		}
		if(((address-y_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0xFD: //SBC_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			SBC();
		}
		if(((address-x_reg)/256)==(address/256))
		{
			if(curr_cycle++==4)curr_inst=0x80;
		}
		else if(curr_cycle++==5)curr_inst=0x80;
		break;
	case 0xFE: //INC_Abs_X();
		if(curr_cycle==1)
		{
			Abs_X();
			Poke(address,gen1=Peek(address)+1);
			setflags(gen1);
		}
		if(curr_cycle++==6) curr_inst=0x80;
		break;
		
		
		// UNDOC'D INSTRUCTIONS ///////////
	case 0x82:	// SKB
	case 0xc2:
	case 0xe2:
	case 0x04:
	case 0x14:
	case 0x34:
	case 0x44:
	case 0x54:
	case 0x64:
	case 0x74:
	case 0x89: // op tests call it NOP #imm ??
	case 0xD4:
	case 0xf4:	curr_loc+=2;	curr_inst=0x80;	break;
#ifdef UNDOCd
	case 0x03: //ASO_Ind_X
		if(curr_cycle==1)
		{
			IndX();
			ASO();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0x07: //ASO_Zero
		if(curr_cycle==1)
		{
			Zero();
			ASO();
		}
		if(curr_cycle++==5)
			curr_inst=0x80;
		break;
#endif
	case 0x0B:	//ASO_Imm
		if(curr_cycle==1)
		{
			Imm();
			gen1=Peek(address);
			if(gen1&0x80)
				flags|=FLAG_CARRY;
			gen1<<=1;
			a_reg|=gen1;
			setflags(a_reg);
		}
		if(curr_cycle++==4)
			curr_inst=0x80;
		break;
#ifdef UNDOCd
	case 0x0F:	Abs(); ASO();	break;
	case 0x13:	Ind_Y(); ASO();	break;
	case 0x17:	//ASO_Zero_X
		if(curr_cycle==1)
		{
			Zero_X();
			ASO();
		}
		if(curr_cycle++==6)
			curr_inst=0x80;
		break;
	case 0x1B:	Abs_Y();	ASO();	break;
#endif
	case 0x1F:
		if(curr_cycle==1)
		{
			Abs_X();
			ASO();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
#ifdef UNDOCd
	case 0x23:	//RLA_IndX
		if(curr_cycle==1)
		{
			IndX();
			RLA();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0x27:	//RLA_Zero
		if(curr_cycle==1)
		{
			Zero();
			RLA();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0x2B:
		if(curr_cycle==1)
		{
			Imm();
			RLA();
		}
		if(curr_cycle++==6)
			curr_inst=0x80;
		break;
	case 0x2F:
		if(curr_cycle==1)
		{
			Abs();
			RLA();
		}
		if(curr_cycle++==7)
			curr_inst=0x80;
		break;
	case 0x33:
		if(curr_cycle==1)
		{
			Ind_Y();
			RLA();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0x37:
		if(curr_cycle==1)
		{
			Zero_X();
			RLA();
		}
		if(curr_cycle++==7)
			curr_inst=0x80;
		break;
	case 0x3B:
		if(curr_cycle==1)
		{
			Abs_Y();
			RLA();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0x3F:
		if(curr_cycle==1)
		{
			Abs_X();
			RLA();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
#endif
		//case 0x43:	IndX();	LSE();	break;
		//case 0x47:	Zero();	LSE();	break;
		//case 0x4B:	Imm();	ALR();	break;//
		//case 0x4F:	Abs();	LSE();	break;
		//case 0x53:	Ind_Y();	LSE();	break;
		//case 0x57:	Zero_X();	LSE();	break;
		//case 0x5B:	Abs_Y();LSE();	break;
		//case 0x5F:	Abs_X();	LSE();	break; //
		//case 0x63:	IndX();	RRA();	break;
	case 0x67:
		if(curr_cycle==1)
		{
			Zero();
			RRA();
		}
		if(curr_cycle++==5)
			curr_inst=0x80;
		break;
		//case 0x6B:	Imm();	ARR();	break; //
		//case 0x6F:	Abs();	RRA();	break;
		//case 0x73:	Ind_Y();	RRA();	break;
		//case 0x77:	Zero_X();	RRA();	break;
		//case 0x7B:	Abs_Y();	RRA();	break;
		//case 0x7F:	Abs_X();	RRA();	break;
		//case 0x83:	IndX();	AXS();	break;
	case 0x87:
		if(curr_cycle==1)
		{
			Zero();
			AXS();
		}
		if(curr_cycle++==5)
			curr_inst=0x80;
		break;
		//case 0x8F:	Abs();	AXS();	break;
		//case 0x97:	Zero_Y();	AXS();	break;
		//case 0xA3:	IndX();	LAX();	break;
		//case 0xA7:	Zero();	LAX();	break;
		//case 0xAB:	Imm();	OAL();	break;
		//case 0xAF:	Abs();	LAX();	break;
		//case 0xB3:	Ind_Y();	LAX();	break;
		//case 0xB7:	Zero_X();	LAX();	break;
	case 0xBF:
		if(curr_cycle==1)
		{
			Abs_Y();
			LAX();
		}
		if(curr_cycle==1)
			curr_inst=0x80;
		break;
	case 0xC3:
		if(curr_cycle==1)
		{
			IndX();
			DCM();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0xC7:
		if(curr_cycle==1)
		{
			Zero();
			DCM();
		}
		if(curr_cycle++==6)
			curr_inst=0x80;
		break;
		//case 0xCB:	Imm();		SAX();	break;
		//case 0xCF:	Abs();		DCM();	break;
		//case 0xD3:	Ind_Y();	DCM();	break;
		//case 0xD7:	Zero_X();	DCM();	break;
		//case 0xDB:	Abs_Y();	DCM();	break;
		//case 0xDF:	Abs_X();	DCM();	break;
		//case 0xE3:	IndX();		INS();	break;
		//case 0xE7:	Zero();		INS();	break;
		//case 0xEF:	Abs();		INS();	break;
		//case 0xF3:	Ind_Y();	INS();	break;
		//case 0xF7:	Zero_X();	INS();	break;
		//case 0xFB:	Abs_Y();	INS();	break;
	case 0xFF:
		if(curr_cycle==1)
		{
			Abs_X();
			INS();
		}
		if(curr_cycle++==8)
			curr_inst=0x80;
		break;
	case 0x0C:	// SKW
	case 0x1C:
	case 0x3C:
	case 0x5C:
	case 0x7C:
	case 0xDC:
	case 0xFC:	curr_loc+=3;	curr_inst=0x80;	break;
	default:
/*		if(illopc)
				Processor_hung(curr_loc,curr_inst,jmp_from);
*/		curr_loc++;
		curr_inst=0x80;
	}// end switch
}