/*=============================================================================
    OLEditorProto.h — binary protocol between OLGame and the native Linux editor.

    Scene state is broadcast via UDP on 127.0.0.1:27668 every editor tick.
    The editor reads it and renders the scene panel, gizmos, etc.
    IPC commands (OL_SETVIEW, SELECTNAME, etc.) continue on TCP port 27666.
=============================================================================*/
#pragma once

#pragma pack(push, 1)

// Max actors per packet — increase if needed
#define OL_PROTO_MAX_ACTORS 150  // fits in one UDP datagram (~56KB)
#define OL_PROTO_MESH_NAME  64
#define OL_PROTO_ACTOR_NAME 64
#define OL_PROTO_CLASS_NAME 48
#define OL_PROTO_TAG_NAME   32

#define OL_PROTO_MAGIC   0x4F4C4544  // "OLED"
#define OL_PROTO_VERSION 1

struct OLProto_Vec3  { float x, y, z; };
struct OLProto_Rot   { int pitch, yaw, roll; }; // Unreal rotation units (65536=360°)
struct OLProto_Box   { OLProto_Vec3 min, max; };

// Flags
#define OL_ACTOR_SELECTED  (1<<0)
#define OL_ACTOR_HIDDEN    (1<<1)
#define OL_ACTOR_HIDDEN_ED (1<<2) // bHiddenEdLayer

struct OLProto_Actor
{
    char         name[OL_PROTO_ACTOR_NAME];
    char         cls[OL_PROTO_CLASS_NAME];
    char         tag[OL_PROTO_TAG_NAME];
    char         mesh[OL_PROTO_MESH_NAME]; // StaticMesh name or empty
    OLProto_Vec3 location;
    OLProto_Rot  rotation;
    OLProto_Vec3 scale3d;
    float        draw_scale;
    OLProto_Box  bounds;
    unsigned int flags; // OL_ACTOR_* bitmask
};

struct OLProto_Camera
{
    OLProto_Vec3 location;
    OLProto_Rot  rotation;
    float        fov;       // degrees
    float        aspect;    // width/height
};

struct OLProto_Header
{
    unsigned int magic;     // OL_PROTO_MAGIC
    unsigned int version;   // OL_PROTO_VERSION
    unsigned int frame_id;  // increments each send
    unsigned int num_actors;
    OLProto_Camera camera;
    // Followed by num_actors × OLProto_Actor
};

#pragma pack(pop)
