#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <Windows.h>

static void StartRead();
static void StopRead();

static void StartWrite();
static void StopWrite();

static void InitEvents();
static void FreeEvents();
static void InitThreads();

DWORD WINAPI Write(LPVOID);
DWORD WINAPI Read(LPVOID);

CONST DWORD writerDelayMax = 500; // msecs
CONST DWORD readerDelayMax = 300;

CONST BYTE writersQty = 3;
CONST BYTE readersQty = 5;

CONST BYTE recsPerThread = 3;

volatile LONG activeReaders = 0;
volatile LONG waitingReaders = 0;
volatile LONG waitingWriters = 0;

HANDLE permittedWrite;
HANDLE permittedRead;
HANDLE mutex;

HANDLE writers[writersQty];
HANDLE readers[readersQty];

BOOL isWriting = FALSE;

INT value = 0;

int main()
{
    if (!(mutex = CreateMutex(NULL, FALSE, NULL)))
    {
        perror("CreateMutex");
        exit(1);
    }

    InitEvents();
    InitThreads();

    WaitForMultipleObjects(writersQty, writers, TRUE, INFINITE);
    WaitForMultipleObjects(readersQty, readers, TRUE, INFINITE);

    FreeEvents();

    CloseHandle(mutex);

    return 0;
}

void InitEvents()
{
    permittedWrite = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (permittedWrite == NULL)
    {
        perror("CreateEvent, permittedWrite");
        exit(1);
    }

    permittedRead = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (permittedRead == NULL)
    {
        perror("CreateEvent, permittedRead");
        exit(1);
    }
}

void FreeEvents()
{
    if (!CloseHandle(permittedWrite))
    {
        perror("CloseHandle, permittedWrite");
        exit(1);
    }

    if (!CloseHandle(permittedRead))
    {
        perror("CloseHandle, permittedRead");
        exit(1);
    }
}

void InitThreads()
{
    BYTE idx;

    for (idx = 0; idx < writersQty; idx++)
    {
        writers[idx] = CreateThread(NULL, 0, &Write, NULL, 0, NULL);

        if (writers[idx] == NULL)
        {
            fprintf(stderr, "CreateThread, writer %d: %s\n",
                    idx, strerror(errno));
            exit(1);
        }
    }

    for (idx = 0; idx < readersQty; idx++)
    {
        readers[idx] = CreateThread(NULL, 0, &Read, NULL, 0, NULL);

        if (readers[idx] == NULL)
        {
            fprintf(stderr, "CreateThread, reader %d: %s\n",
                    idx, strerror(errno));
            exit(1);
        }
    }
}

void StartWrite()
{
    InterlockedIncrement(&waitingWriters);

    if (isWriting || activeReaders > 0)
    {
        WaitForSingleObject(permittedWrite, INFINITE);
    }
    WaitForSingleObject(mutex, INFINITE);

    InterlockedDecrement(&waitingWriters);
    ResetEvent(permittedWrite);
    isWriting = TRUE;

    ReleaseMutex(mutex);
}

void StopWrite()
{
    isWriting = FALSE;

    if (waitingReaders > 0)
    {
        SetEvent(permittedRead);
    }
    else
    {
        SetEvent(permittedWrite);
    }
}

void StartRead()
{
    InterlockedIncrement(&waitingReaders);

    if (isWriting || waitingWriters > 0)
    {
        WaitForSingleObject(permittedRead, INFINITE);
    }

    InterlockedDecrement(&waitingReaders);
    InterlockedIncrement(&activeReaders);

    SetEvent(permittedRead);
}

void StopRead()
{
    InterlockedDecrement(&activeReaders);

    if (activeReaders == 0)
    {
        SetEvent(permittedWrite);
    }
}

DWORD WINAPI Write(LPVOID)
{
    srand(time(NULL) ^ GetCurrentThreadId());

    for (BYTE i = 0; i < recsPerThread; i++)
    {
        StartWrite();

        printf("Writer %6d: %3d -> %3d\n",
               GetCurrentThreadId(), value++, value);

        StopWrite();

        Sleep(rand() % writerDelayMax);
    }

    return 0;
}

DWORD WINAPI Read(LPVOID)
{
    srand(time(NULL) ^ GetCurrentThreadId());

    BOOL isWritingCompleted = FALSE;
    while (!isWritingCompleted)
    {
        StartRead();

        printf("Reader %6d: %3d\n",
               GetCurrentThreadId(), value);
        isWritingCompleted = value >= writersQty * recsPerThread;

        StopRead();

        Sleep(rand() % readerDelayMax);
    }

    return 0;
}

