
#include <DiskDrm.h>

INT64 DiskInfo(VOID) {
    INT64 DISKCount = 1, i = 0;
    EFI_HASH2_PROTOCOL *mCryptoProtocol;
    EFI_STATUS Status;

    Status = HashSetup(&mCryptoProtocol);
    if (EFI_ERROR (Status)) {
        Print(L"Error when initializing Hash2 protocol: %r\n", Status);
        return EFI_ABORTED;
    }

    Print(L"DISK Storage\n");
    for (i = 0;; i++) {
        EFI_BLOCK_IO_PROTOCOL *BlkIo;
        CHAR16 Desc[DISK_DESCRIPTION_SIZE + 1];
        UINT8 HashedInfo[MAX_HASH_CTX_SIZE + 1];
        UINTN HashCtxSize;

        if ((i = DISK_Iterator(i, &BlkIo, Desc, DISK_DESCRIPTION_SIZE)) < 0) {
            break;
        }

        Status = HashInfo(Desc, mCryptoProtocol, &gEfiHashAlgorithmSha256Guid, HashedInfo, &HashCtxSize);
        if (EFI_ERROR (Status)) {
            Print(L"Error when hashing description of DISK #%d: %r\n", DISKCount, Status);
            return EFI_ABORTED;
        }

        Print(L"  DISK #%d:\n", DISKCount);
        Print(L"  Description  : %s\n", Desc);
        Print(L"  Size : %d blocks = %ld bytes\n", BlkIo->Media->LastBlock + 1,
              (BlkIo->Media->LastBlock + 1) * BlkIo->Media->BlockSize);
        Print(L"  SHA256 : ");
        PrintHash(HashedInfo, HashCtxSize);

        DISKCount++;
    }
    if (!DISKCount) {
        Print(L"  Not found.\n");
    }
    return DISKCount;
}
