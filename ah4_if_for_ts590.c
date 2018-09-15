/*
 * Copyright (c) 2012
 *      Tamotsu Kanoh <kanoh@kanoh.org>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither my name nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <pic.h>

#define	N_TS		GPIO5
#define	N_TT		GPIO4
#define	KEY		GPIO0
#define	TUNE		GPIO1
#define	N_MANUAL	GPIO2

__CONFIG(
	FOSC_INTRCIO &
	WDTE_OFF &
	PWRTE_OFF &
	MCLRE_OFF &
	BOREN_OFF &
	CP_OFF &
	CPD_OFF
);

__IDLOC(F629);

int td;

void interrupt ISR(void)
{
	if(TMR1IF) {
		td++;
		TMR1H	= 0x3c;
		TMR1L	= 0xb0;
		TMR1IF = 0;
	}
}

void main(void)
{
	int lock,phase,manual;

	INTCON	= 0x00;
	CMCON	= 0x07;
	TRISIO	= 0x2d;

	T1CON	= 0x10;
	TMR1ON	= 0;
	TMR1IE	= 0;
	PEIE	= 0;
	GIE = 0;
	TMR1H	= 0x3c;
	TMR1L	= 0xb0;

	lock = 0;
	manual = 0;
	phase = 0;
	td =0;
	
	N_TT = 1;
	TUNE = 0;

	while(1) {
		if(lock) {	
			if(phase == 1 && td == 1) {
				TUNE = 1;
				phase = 2;
			}
			else if(phase == 2 && td > 2) {	
				if(KEY) {
					TUNE = 0;	
					phase = 3;
				}
			}
			else if(phase == 2 && td > 10) {
					if(!manual) N_TT = 0;
					phase = 4;
			}
			else if(phase == 3 && td > 10) {
				if(manual) {
					if(!KEY || td > 140) phase = 4;
				}
				else {
					if(!KEY || N_TS || td > 140) {
						N_TT = 0;
						phase = 4;
					}
				}
			}
			else if(phase == 4 && N_TS) {
				TMR1ON	= 0;
				TMR1IE	= 0;
				TMR1H	= 0x3c;
				TMR1L	= 0xb0;				
				PEIE	= 0;
				GIE = 0;
				N_TT = 1;
				TUNE = 0;
				lock  = 0;
				manual = 0;
				phase = 0;
				td = 0;
			}
		}
		else {
			if(!N_TS) {
				TUNE = 0;
				N_TT = 1;
				lock = 1;
				manual = 0;
				phase = 1;
				td = 0;
				TMR1H	= 0x3c;
				TMR1L	= 0xb0;		
				TMR1ON	= 1;
				TMR1IE	= 1;
				PEIE = 1;
				GIE = 1;
			}
			else if(!N_MANUAL) {
				TUNE = 0;
				N_TT = 1;
				lock = 1;
				manual = 1;
				phase = 1;
				td = 0;
				TMR1H	= 0x3c;
				TMR1L	= 0xb0;		
				TMR1ON	= 1;
				TMR1IE	= 1;
				PEIE = 1;
				GIE = 1;
			}
		}
	}
}
