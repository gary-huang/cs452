#ifndef HANDLER_INTERRUPT_H
#define HANDLER_INTERRUPT_H

#include <kernel.h>
#include <interrupt_matrix.h>

#define VIC1_ENABLED 0
#define VIC2_ENABLED (1 << 19)

void init_irq( interrupt_matrix *im );
void cleanup_irq();
void event_register( interrupt_matrix *im, TaskDescriptor *td);
void event_wake(interrupt_matrix *im);


#endif /*HANDLER_INTERRUPT_H*/
