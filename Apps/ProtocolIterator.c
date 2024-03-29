#include <DiskDrm.h>

// Note: not thread-safe yet.
INT64 BlkIo_Iterator(INT64 index, EFI_HANDLE *Handle, EFI_BLOCK_IO_PROTOCOL **BlkIo)
{
  static INT64 HandleCount=0;
  static EFI_HANDLE *HandleBuffer = NULL;
  EFI_STATUS Status;

  if (index == 0) {
    SafeFreePool((void**)&HandleBuffer);
    HandleCount = 0;
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, (UINTN *)&HandleCount, &HandleBuffer);
    if (EFI_ERROR(Status)) {
      Print(L"Error when locating BLOCK IO: %r\n", Status);
      return BlkIo_Error;
    }
    else {
      if (Verbose_Level > 1) {
        Print(L"BlkIo_Iterator(): %d instance(s).\n", HandleCount);
      }
    }
  }
  else if (index >= HandleCount || !HandleBuffer || !HandleCount) {
      SafeFreePool((void**)&HandleBuffer);
      HandleCount = 0;
    return BlkIo_Terminate;
  }

  for (; index<HandleCount; index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) BlkIo
                    );
    if (EFI_ERROR (Status) || (*BlkIo)->Media->LogicalPartition) {
      continue;
    }
    *Handle = HandleBuffer[index];
    return index;
  }
  SafeFreePool((void**)&HandleBuffer);
  HandleCount = 0;
  return BlkIo_Terminate;
}


// Note: not thread-safe yet.
INT64 DISK_Iterator(INT64 index, EFI_BLOCK_IO_PROTOCOL **BlkIo, CHAR16 *Desc_Buffer, INT16 Desc_Max)
{
  INT64 i=0;

  for (i=index;;i++) {
    EFI_HANDLE Handle;
    CHAR16 *Description;

    if ((i = BlkIo_Iterator(i, &Handle, BlkIo)) < 0) {
      break;
    }

    Description = BmGetBootDescriptionMinimal(Handle);
    if (!Description) {
        continue;
    }
    StrnCpy(Desc_Buffer, Description, Desc_Max);
    Desc_Buffer[Desc_Max-1] = L'\0';

    SafeFreePool((void**)&Description);
    break;
  }
  return i;
}