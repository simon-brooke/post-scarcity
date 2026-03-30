

#import "memory/pointer.h"
#import "memory/pso.h"
#import "payloads/exception.h"

/**
 * @param p a pointer to an object.
 * @return true if that object is an exception, else false.
 */
bool exceptionp( struct pso_pointer p) {
	return (get_tag_value( p) == EXCEPTIONTV);
}
