#pragma once
#include "EngineCommon/include/types.h"

struct Arena
{

};

Arena* ArenaAlloc();
void ArenaRelease(Arena* arena);
void ArenaSetAutoAlign(Arena* arena, u64 align);

u64 ArenaPos(Arena* arena);

void* ArenaPushNoZero(Arena* arena, u64 size);
void* ArenaPushAligner(Arena* arena, u64 alignment);
void* ArenaPush(Arena* arena, u64 size);

void ArenaPopTo(Arena* arena, u64 pos);
void ArenaPop(Arena* arena, u64 size);
void ArenaClear(Arena* arena);

#define PushArrayNoZero(arena, type, count)  reinterpret_cast<type*>(ArenaPushNoZero((arena), sizeof(type)*(count)))
#define PushArray(arena, type, count) reinterpret_cast<type*>(ArenaPush((arena), sizeof(type)*(count)))
