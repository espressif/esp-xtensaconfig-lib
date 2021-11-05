#ifndef XTENSA_CONFIG_FIXUP_H
#define XTENSA_CONFIG_FIXUP_H

// INCLUDES FIXUP
#define _GL_ALREADY_INCLUDING_STRING_H

// gdb's config.h has not this definition needed for unistd.h
#if !defined(_GL_INLINE_HEADER_BEGIN) || !defined(_GL_INLINE_HEADER_END)
#if __GNUC__ == 4 && 6 <= __GNUC_MINOR__
# if defined __GNUC_STDC_INLINE__ && __GNUC_STDC_INLINE__
#  define _GL_INLINE_HEADER_CONST_PRAGMA
# else
#  define _GL_INLINE_HEADER_CONST_PRAGMA \
     _Pragma ("GCC diagnostic ignored \"-Wsuggest-attribute=const\"")
# endif
# define _GL_INLINE_HEADER_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wmissing-prototypes\"") \
    _Pragma ("GCC diagnostic ignored \"-Wmissing-declarations\"") \
    _GL_INLINE_HEADER_CONST_PRAGMA
# define _GL_INLINE_HEADER_END \
    _Pragma ("GCC diagnostic pop")
#else
# define _GL_INLINE_HEADER_BEGIN
# define _GL_INLINE_HEADER_END
#endif
#endif //!defined(_GL_INLINE_HEADER_BEGIN) || !defined(_GL_INLINE_HEADER_END)

#endif // XTENSA_CONFIG_FIXUP_H
