#include <windows.h>

#include <stdio.h>
#include <winnt.h>

static union PI {
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info;
  char buf[1024 * 1024];
} pi_buf = {};

const char *rel_str[] = {
    "ProcCore", "NumaNode", "Cache",    "ProcPkg",
    "Group",    "ProcDie",  "NumaNoEx", "ProcMod",
};

static void dump_affinity(const GROUP_AFFINITY *a, int count) {
  for (int c = 0; c < count; c++) {
    if (c) {
      printf("                                                ");
    }
    printf("%d G=%d M=0x%08.8zx:%08.8zx ", c, a->Group, a->Mask >> 32,
           a->Mask & 0xFFFFFFFF);
    for (int i = sizeof(a->Mask) * 8 - 1; i >= 0; i--) {
      printf("%zu", (a->Mask >> i) & 1);
      if (i && i % 32 == 0)
        printf(":");
    }
    printf("\n");
  }
}

static void dump_group_info(const PROCESSOR_GROUP_INFO *a, int count) {
  for (int c = 0; c < count; c++) {
    if (c) {
      printf("                         ");
    }
    printf("   %d MaxProc=%-3d ActProc=%-3d M=0x%08.8zx:%08.8zx ", c,
           a->MaximumProcessorCount, a->ActiveProcessorCount,
           a->ActiveProcessorMask >> 32, a->ActiveProcessorMask & 0xFFFFFFFF);
    for (int i = sizeof(a->ActiveProcessorMask) * 8 - 1; i >= 0; i--) {
      printf("%zu", (a->ActiveProcessorMask >> i) & 1);
      if (i && i % 32 == 0)
        printf(":");
    }
    printf("\n");
  }
}

int main() {
  DWORD length = sizeof(pi_buf);
  if (!GetLogicalProcessorInformationEx(RelationAll, &pi_buf.info, &length)) {
    printf("GetLogicalProcessorInformationEx: Return buffer too large: %d\n",
           length);
    return -1;
  }

  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *pi = &pi_buf.info;
  int index = 0;
  int totalSize = 0;
  for (;;) {
    if (pi->Size == 0)
      break;
    if (pi->Relationship >= 0 && pi->Relationship <= RelationProcessorModule) {
      printf("%-8s ", rel_str[pi->Relationship]);
      switch (pi->Relationship) {
      case RelationProcessorCore:
      case RelationProcessorPackage:
      case RelationProcessorDie:
      case RelationProcessorModule:
        printf("F=%-2d EC=%-2d GC=%-2d                       ",
               pi->Processor.Flags, pi->Processor.EfficiencyClass,
               pi->Processor.GroupCount);
        dump_affinity(pi->Processor.GroupMask, pi->Processor.GroupCount);
        break;
      case RelationNumaNode:
      case RelationNumaNodeEx:
        printf("N=%-2d GC=%-2d                             ",
               pi->NumaNode.NodeNumber, pi->NumaNode.GroupCount);
        dump_affinity(pi->NumaNode.GroupMasks, pi->NumaNode.GroupCount);
        break;
      case RelationCache:
        printf("L=%-2d A=%-2d LS=%-3d CS=%-8d T=%d GC=%-2d ", pi->Cache.Level,
               pi->Cache.Associativity, pi->Cache.LineSize, pi->Cache.CacheSize,
               pi->Cache.Type, pi->Cache.GroupCount);
        dump_affinity(pi->Cache.GroupMasks, pi->Cache.GroupCount);
        break;
      case RelationGroup:
        printf("MaxGC=%d ActGC=%d ", pi->Group.MaximumGroupCount,
               pi->Group.ActiveGroupCount);
        dump_group_info(pi->Group.GroupInfo, pi->Group.ActiveGroupCount);
        break;
      }
    } else {
      printf("UNKNOWN %d rel=%d size=%d", index, pi->Relationship, pi->Size);
    }
    pi = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)((const char *)pi +
                                                     pi->Size);
    totalSize += pi->Size;
    index++;
  }
  printf("\n  Legend:\n\n");
  printf("  F=ProcFlags EC=ProcEfficiencyClass GC=GroupCount N=NodeNumber\n");
  printf("  L=CacheLevel A=CacheAssoc LS=LineSize CS=CacheSize T=CacheType\n");
  printf("                       T: 0=Unified 1=Instruction 2=Data 3=Trace\n");
}
