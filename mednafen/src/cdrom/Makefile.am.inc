libmednafen_marley_a_SOURCES	+=	cdrom/crc32.cpp cdrom/galois.cpp cdrom/l-ec.cpp cdrom/recover-raw.cpp cdrom/lec.cpp
libmednafen_marley_a_SOURCES	+=	cdrom/CDUtility.cpp
libmednafen_marley_a_SOURCES	+=	cdrom/CDInterface.cpp cdrom/CDInterface_MT.cpp cdrom/CDInterface_ST.cpp
libmednafen_marley_a_SOURCES	+=	cdrom/CDAccess.cpp cdrom/CDAccess_Image.cpp cdrom/CDAccess_CCD.cpp

libmednafen_marley_a_SOURCES	+=	cdrom/CDAFReader.cpp
libmednafen_marley_a_SOURCES	+=	cdrom/CDAFReader_Vorbis.cpp
libmednafen_marley_a_SOURCES	+=	cdrom/CDAFReader_MPC.cpp

if HAVE_LIBSNDFILE
libmednafen_marley_a_SOURCES	+=	cdrom/CDAFReader_SF.cpp
endif

libmednafen_marley_a_SOURCES	+=	cdrom/scsicd.cpp

