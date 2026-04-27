#include <windows.h>

#include <stdio.h>
#include <winnt.h>

static union PI {
   SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info;
   char buf[1024*1024]; 
} pi_buf = {};

const char* rel_str[] = {
    "ProcessorCore",
    "NumaNode",
    "Cache",
    "ProcessorPackage",
    "Group",
    "ProcessorDie",
    "NumaNodeEx",
    "ProcessorModule",
};

int main()
{
    DWORD length = sizeof(pi_buf);
    if( !GetLogicalProcessorInformationEx(RelationAll, &pi_buf.info, &length) )
    {
        printf("Return buffer too large: %d\n", length);
        return -1;
    }

    printf("Returned buffer: %d\n", length);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* pi = &pi_buf.info; 
    int index = 0;
    int totalSize = 0;
    for( ;; )
    {
        if( pi->Size == 0 )
            break;
        if( pi->Relationship >= 0 && pi->Relationship <= RelationProcessorModule )
        {
            printf("#%-3d rel=%-2d size=%-5d %-16s: ", index, pi->Relationship, pi->Size, rel_str[pi->Relationship]);
            switch( pi->Relationship )
            {
                case RelationProcessorCore:
                case RelationProcessorPackage:
                case RelationProcessorDie:
                case RelationProcessorModule:
                    printf("  PROCESSOR: Flags=%d EffCls=%d GroupCount=%d\n", pi->Processor.Flags, pi->Processor.EfficiencyClass, pi->Processor.GroupCount );
                    break;
                case RelationNumaNode:
                case RelationNumaNodeEx:
                    printf("  NUMA_NODE: Node=%d GroupCount=%d\n", pi->NumaNode.NodeNumber, pi->NumaNode.GroupCount );
                    break;
                case RelationCache:
                    printf("  CACHE: Level=%d Assoc=%d LineSize=%d CacheSize=%d Type=%d GroupCount=%d\n", pi->Cache.Level, pi->Cache.Associativity, pi->Cache.LineSize, pi->Cache.CacheSize, pi->Cache.Type, pi->Cache.GroupCount );
                    break;
                case RelationGroup:
                    printf("  GROUP: MaxGroupCount=%d ActiveGroupAcount=%d\n", pi->Group.MaximumGroupCount, pi->Group.ActiveGroupCount );
                    break;
            }
        }
        pi = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) ((const char*)pi + pi->Size);
        totalSize += pi->Size;
        index ++;
    }
}