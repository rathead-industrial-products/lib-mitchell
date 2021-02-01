Lockless Data Structures {#faq_page_title}
===

\tableofcontents

# Semaphore    {#lockless_semaphore}
Semaphore Description.

# Queue   {#lockless_queue}
A queue is a fixed-sized ring buffer. Each queue element is a type (void *),
so it may be cast to either a pointer of some type or a 32 bit value. Each
entry has an associated valid bit. Values are inserted at the head and removed
from the tail. While the head and tail ring indices are always updated
atomically, the reading and writing of the values are protected by the valid
bit and may therefore be interrupted. The head pointer always points to an
invalid entry. This means a queue always has one entry that cannot be used.

 <img src="queue.png" align="left" height="500"> 



