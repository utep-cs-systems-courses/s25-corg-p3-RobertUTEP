        .text
        .global nextMode
        .type   nextMode, @function
nextMode:
        ; r12 = argument m, return in r12
        add     #1, r12
        and     #3, r12
        ret
