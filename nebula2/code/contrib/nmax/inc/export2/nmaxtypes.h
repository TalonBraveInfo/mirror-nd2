#ifndef N_MAXTYPES_H
#define N_MAXTYPES_H

// paramblock2 block and parameter IDs for the standard shaders
// NB these are duplicated in shaders/stdShaders.cpp...
enum {
    SHDR_PARAMS,
};

// shdr_params param IDs
enum {
    SHDR_AMBIENT,
    SHDR_DIFFUSE,
    SHDR_SPECULAR,
    SHDR_AD_TEXLOCK,
    SHDR_AD_LOCK,
    SHDR_DS_LOCK,
    SHDR_USE_SELF_ILLUM_COLOR,
    SHDR_SELF_ILLU_AMNT,
    SHDR_SELF_ILLUM_COLOR,
    SHDR_SPEC_LVL,
    SHDR_GLOSSINESS,
    SHDR_SOFTEN,

};

#endif
 