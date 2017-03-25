enable_language(ASM-ATT)
aux_source_directory(. src)
aux_source_directory(stdio stdio)
aux_source_directory(stdlib stdlib)
add_library(c 
    crt0.s
    ${src}
    ${stdio}
    ${stdlib}
    )
