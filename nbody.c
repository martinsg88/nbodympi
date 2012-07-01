#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WIDTH  1000
#define HEIGHT 1000
#define BODIES 100
#define GRAVITY .002
#define TIMESTEP 1000


typedef enum {FALSE, TRUE} boolean;

typedef struct particle {
    double x, y, m, xv, yv;
} particle;

typedef struct forces {
    double x, y;
} forces;

typedef struct acceleration {
    double x, y;
} acceleration;

int main() {
    FILE *fout;
    char buffer[50];
    int t, i, j, x, y;
    double f, dx, dy, r, fx, fy, theta;
	struct timeval tim1, tim2;
	
	gettimeofday(&tim1, NULL); 
	double t1 = tim1.tv_sec+(tim1.tv_usec/1000000.0);
	
	srand(time(0));	
    particle particles[BODIES];
    for (i = 0; i < BODIES; i++) {
        particles[i] = (particle){
            random() % WIDTH,
            random() % HEIGHT,
            random() % 900,
            0, 0,
        };
    }
    particle new_particles[BODIES] = {};
    forces new_forces[BODIES] = {};
    acceleration accels[BODIES] = {};
    boolean is_body;

    for (t = 0; t < TIMESTEP; t++) {
        for (i = 0; i < BODIES; i++) {
            fy, fx = 0;
            new_particles[i] = particles[i];
            for (j = 0; j < BODIES; j++) {
                if (j != i) {
                    dy = particles[j].y - particles[i].y;
                    dx = particles[j].x - particles[i].x;
                    r = sqrt((dy * dy) + (dx * dx));
                    f = (GRAVITY * particles[i].m * particles[j].m) / (r * r);
                    theta = atan2(dy, dx);
                    new_forces[i].y += f * sin(theta);
                    new_forces[i].x += f * cos(theta);
                }
            }
            accels[i].y = new_forces[i].y / particles[i].m;
            accels[i].x = new_forces[i].x / particles[i].m;
            new_particles[i].yv += accels[i].y;
            new_particles[i].xv += accels[i].x;
            new_particles[i].y += new_particles[i].yv;
            new_particles[i].x += new_particles[i].xv;
        }

        for (i = 0; i < BODIES; i++) {
            particles[i] = new_particles[i];
        }		
		
        sprintf(buffer, "%d.pbm", t);
        printf("> %s\n", buffer);
        fout = fopen(buffer, "w");
        fprintf(fout, "P1\n");
        fprintf(fout, "%d %d\n", HEIGHT, WIDTH);
        for (y = 0; y < HEIGHT; y++) {
            for (x = 0; x < WIDTH; x++) {
                is_body = FALSE;
                for (i = 0; i < BODIES; i++) {
                    j = particles[i].m / 100;
                    if (((int)(particles[i].x - j) < x) &&
                            ((int)(particles[i].x + j) > x) &&
                            ((int)(particles[i].y - j) < y) &&
                            ((int)(particles[i].y + j) > y)) {
                        is_body = TRUE;
                    }
                }
                if (is_body)
                    fprintf(fout, "1 ");
                else
                    fprintf(fout, "0 ");
            }
            fprintf(fout, "\n");
        }
        fclose(fout);

    }
	gettimeofday(&tim2, NULL);
	double t2 = tim2.tv_sec+(tim2.tv_usec/100000.0);
	printf("                Sequential took: Total time taken in seconds: %f\n",t2-t1);

    return 0;
}
