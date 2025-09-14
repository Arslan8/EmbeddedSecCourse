typedef void (*ISR)(void);
extern unsigned int _estack;
int global = 0x1234;
void main(); 
__attribute__((section(".isr_vector")))
ISR g_pfnVectors[] = {
    (ISR)&_estack,        // Initial stack pointer
    main,        // Reset handler
};


void main() {
		while(1);
}

