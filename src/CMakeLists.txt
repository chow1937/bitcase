# add header include directories
INCLUDE_DIRECTORIES (.)

# add executable target
ADD_EXECUTABLE (bitcase bitcase.c cmd.c cron.c db.c hashtable.c lru.c bcmem.c)
TARGET_LINK_LIBRARIES (bitcase uv)

# install executable target
INSTALL (TARGETS bitcase DESTINATION "bin")
