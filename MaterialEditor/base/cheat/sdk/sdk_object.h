#pragma once


#define REMOVE_COPY_CONSTRUCTOR(_class) \
_class(const _class&) = delete; \
_class& operator=(const _class&) = delete

#define REMOVE_MOVE_CONSTRUCTOR(_class) \
_class(_class&&) = delete; \
_class& operator=(_class&&) = delete

#define REMOVE_DEFAULT_CONSTRUCTOR(_class) \
_class() = delete; \
private: \
~_class() = default

#define SDK_CLASS(_class) \
REMOVE_COPY_CONSTRUCTOR(_class); \
REMOVE_MOVE_CONSTRUCTOR(_class); \
REMOVE_DEFAULT_CONSTRUCTOR(_class)