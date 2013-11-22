MAIN_SOURCES = \
	src/localtime.h \
	src/localtime.cpp \
	src/NetcdfFile.h \
	src/NetcdfFile.cpp \
	src/VariableConversion.h \
	src/VariableConversion.cpp \
	src/AbstractNetcdfField.h \
	src/NetcdfField.h \
	src/NetcdfField.cpp \
	src/DerivedNetcdfField.h \
	src/DerivedNetcdfField.cpp \
	src/ForceConvertingNetcdfField.h \
	src/ForceConvertingNetcdfField.cpp \
	src/DirectionConvertingNetcdfField.h \
	src/DirectionConvertingNetcdfField.cpp \
	src/WriteQuery.h \
	src/WriteQuery.cpp \
	src/NetcdfTranslator.h \
	src/NetcdfTranslator.cpp \
	src/AbstractDataRetriever.h \
	src/DataRetriever.h \
	src/DataRetriever.cpp \
	src/configuration/CdmLoaderConfiguration.h \
	src/configuration/CdmLoaderConfiguration.cpp \
	src/configuration/LoadElement.h \
	src/configuration/LoadElement.cpp \
	src/configuration/LoadConfiguration.h \
	src/configuration/LoadConfiguration.cpp \
	src/configuration/Conversions.h \
	src/configuration/parameter/DataSpecification.h \
	src/configuration/parameter/DataSpecification.cpp \
	src/configuration/parameter/TimeSpecification.h \
	src/configuration/parameter/TimeSpecification.cpp \
	src/RawData.h


netcdfLoad_SOURCES = \
	src/main.cpp \
	 $(MAIN_SOURCES)
	 
OTHER = \
	src/CdmLoader.h \
	src/CdmLoader.cpp \
	src/NetcdfLoader.h \
	src/NetcdfLoader.cpp 
	


EXTRA_DIST += src/netcdfLoad.mk
