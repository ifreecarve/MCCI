PROJECT(MCCIServer)
 
#what files are needed?
SET(MCCIServer_SRCS
  MCCITypes.h
  FibonacciHeap.h
  MCCIRequestBank.h
  MCCIRequestBanks.h
  MCCITime.h
  MCCIRevisionSet.h
  MCCIRevisionSet.cpp
  MCCIServer.h
  MCCIServer.cpp
  MCCIServerNetworking.h
  MCCISchema.h
  MCCISchema.cpp
  MCCIServerMain.cpp
)

# Add executable 
ADD_EXECUTABLE( MCCIServer ${MCCIServer_SRCS})
 
# indicate how to link
# if rhash and dl don't come at the beginning, it will fail
TARGET_LINK_LIBRARIES(MCCIServer lua5.1 sqlite3 crypto)
