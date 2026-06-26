# Practical Session 4: Shared Memory Programming

**Objective:** Implement inter-process communication using shared memory.

## Files
- `shared_memory.c` — parent process forks a child; parent writes 3 messages into a shared memory segment, child reads them, with named semaphores ensuring the handoff is synchronized

## Key Functions
`shmget`, `shmat`, `shmdt`, `shmctl(IPC_RMID)`, plus `sem_open`/`sem_wait`/`sem_post`/`sem_close`/`sem_unlink` for synchronization

## Build
```bash
gcc -pthread shared_memory.c -o shared_memory
```
> Note: `-pthread` links in the POSIX semaphore implementation on most Linux systems. On some systems you may instead need `-lrt`.

## Run
```bash
./shared_memory
```

## Expected Output
You should see alternating writes and reads, in order, e.g.:
```
[Setup] Shared memory segment created (shmid=...).
[Parent] Wrote: "Message 1 from parent (pid=...)"
[Child]  Read:  "Message 1 from parent (pid=...)" (message_id=1)
[Parent] Wrote: "Message 2 from parent (pid=...)"
[Child]  Read:  "Message 2 from parent (pid=...)" (message_id=2)
[Parent] Wrote: "Message 3 from parent (pid=...)"
[Child]  Read:  "Message 3 from parent (pid=...)" (message_id=3)
[Parent] Shared memory segment removed. Done.
```

## How It Works
1. `shmget()` creates a shared memory segment identified by a key.
2. Both parent and child call `shmat()` to map that segment into their own address space.
3. The parent writes a struct (`message_id` + `text`) into the segment; the child reads it.
4. Two named semaphores (`write_done`, `read_done`) prevent the child from reading stale/partial data and prevent the parent from overwriting data the child hasn't read yet.
5. After all messages are exchanged, the parent calls `shmdt()` to detach and `shmctl(shmid, IPC_RMID, NULL)` to destroy the segment, and both processes unlink the semaphores.

## Applications
Shared memory + semaphores is the foundation of high-performance IPC in Linux — used in database servers, web servers, scientific computing, and real-time systems where copying data between processes (as with pipes or sockets) would be too slow.

## Cleanup Note
If the program crashes before cleanup, you can check for and manually remove leftover shared memory segments with:
```bash
ipcs -m        # list shared memory segments
ipcrm -m <id>  # remove a specific one
```
