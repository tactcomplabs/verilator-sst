#ifndef __TEST_MACROS_H_
#define __TEST_MACROS_H_

#define CHECK_EQ(got, exp) \
    if ((got) != (exp)) { \
        output->fatal(CALL_INFO,1,"GOT = %u EXP = %u\n",got,exp); \
    }

#define CHECK_TRUE(got) \
    if (got == false) { \
        output->fatal(CALL_INFO,1,"GOT = %u, expected truthy value\n",got); \
    }

#endif
