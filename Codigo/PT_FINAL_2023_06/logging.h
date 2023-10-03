#ifndef LOGGING_H_
#define LOGGING_H_

//      ... |xFF|x0F|   |   |   |t7L-t7H|t6L-t6H| ...
//                ^      ^
//   next_pointed_w      next_pointed_t



unsigned int get_next_pointed_temperature_byte(void);
unsigned int get_next_pointed_wet_and_dry_byte(void);
unsigned int get_next_pointed_wet_and_dry_bit(void);
int check_memory_full(void);
//int check_index_max(void);

void wet_and_dry_function(void);
void temperature_function(void);

#endif /* LOGGING_H_ */
