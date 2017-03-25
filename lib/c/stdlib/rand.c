#define RANDOM_MAX 0x7FFFFFFF

static unsigned long next = 1;

int rand(void) {
    long quotient, remainder, t;

    quotient = next / 127773L;
    remainder = next % 127773L;
    t = 16807L * remainder - 2836L * quotient;
    if (t <= 0)
        t += 0x7FFFFFFFL;
    return ((next = t) % ((unsigned long)RANDOM_MAX + 1));
}



void srand(unsigned int seed) {
    next = seed;
}
