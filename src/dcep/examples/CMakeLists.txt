
build_lib_example(
        NAME lazy-migration-our-approach
        SOURCE_FILES lazy-migration-our-approach.cpp ExperimentUtils.cpp
        HEADER_FILES ExperimentUtils.h
        LIBRARIES_TO_LINK
        ${libdcep}
        ${libstream-processing}
        ${libnetanim}
        ${libmobility}
        ${libwifi}
        ${libstats}
        ${libcsma}
        ${libinternet}
        ${libnetwork}
        ${libolsr}
        ${libpoint-to-point}
)


build_lib_example(
        NAME lazy-migration-minimize-latency
        SOURCE_FILES lazy-migration-minimize-latency.cpp ExperimentUtils.cpp
        HEADER_FILES ExperimentUtils.h
        LIBRARIES_TO_LINK
        ${libdcep}
        ${libstream-processing}
        ${libnetanim}
        ${libmobility}
        ${libwifi}
        ${libstats}
        ${libcsma}
        ${libinternet}
        ${libnetwork}
        ${libolsr}
        ${libpoint-to-point}
)


add_custom_target(
        Expose COMMAND ./waf
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/ns-3-dev
)

add_custom_target(
        LazyMigrationExperiments COMMAND ./waf
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/ns-3-dev
)
