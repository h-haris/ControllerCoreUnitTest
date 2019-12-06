/* file: minunit.h */

/* author and source of mu_assert and mu_run_test: http://www.jera.com/techinfo/jtns/jtn002.html */
/* documentation (.pdf): JTN002 - MinUnit -- a minimal unit testing framework for C */

/* To define mu_dbg, see macro DEBUG here: http://c-faq.com/cpp/debugmacs.html */

#define mu_assert(message, test) do { if (!(test)) {mu_dbg("%s",message); return message;} } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)

extern int tests_run;
