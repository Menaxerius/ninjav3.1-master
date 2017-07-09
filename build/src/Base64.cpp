#include "Base64.hpp"
#include "Base64_data.hpp"

#include <stdlib.h>
#include <stdio.h>


uint64_t base64_encode(char** const dest, const char* const str, const uint64_t len) {
    uint64_t i;
    const uint8_t *s = (const uint8_t *) str;
    uint8_t *p;
    
	*dest = (char *)malloc( len * 8 / 6 + 4 );
	if ( *dest == NULL ) {
		perror("Couldn't malloc() space for base64 encoding");
		return 0;
	}
		    
     p = (uint8_t *) *dest;

    /* unsigned here is important! */
    /* uint8_t is fastest on G4, amd */
    /* uint32_t is fastest on Intel */
    uint32_t t1, t2, t3;

    for (i = 0; i < len - 2; i += 3) {
        t1 = s[i]; t2 = s[i+1]; t3 = s[i+2];
        *p++ = e0[t1];
        *p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *p++ = e1[((t2 & 0x0F) << 2) | ((t3 >> 6) & 0x03)];
        *p++ = e2[t3];
    }

    switch (len - i) {
	    case 0:
	        break;
	        
	    case 1:
	        t1 = s[i];
	        *p++ = e0[t1];
	        *p++ = e1[(t1 & 0x03) << 4];
	        *p++ = CHARPAD;
	        *p++ = CHARPAD;
	        break;
	        
	    default: /* case 2 */
	        t1 = s[i]; t2 = s[i+1];
	        *p++ = e0[t1];
	        *p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
	        *p++ = e2[(t2 & 0x0F) << 2];
	        *p++ = CHARPAD;
    }

    *p = '\0';
    
    return (uint64_t)(p - (uint8_t *) *dest);
}
