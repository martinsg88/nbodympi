#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
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

int main(int argc, char* argv[]) {
    FILE *fout;
    char buffer[50];
    int t, i, j, x, y;
    double f, dx, dy, r, fx, fy, theta;
	struct timeval tim1, tim2;
    int size, rank, ierr;
	
	if(rank == 0){
		gettimeofday(&tim1, NULL); 
		double t1 = tim1.tv_sec+(tim1.tv_usec/1000000.0);
	}
	
	gettimeofday(&tim1, NULL); 
	double t1 = tim1.tv_sec+(tim1.tv_usec/1000000.0);
	
	srand(time(0));	
    particle particles[BODIES];
    for (i = 0; i < BODIES; i++){
        particles[i] = (particle){
            random() % WIDTH,
            random() % HEIGHT,
            random() % 500,
            0, 0,
        };
    }
    particle new_particles[BODIES] = {};
    forces new_forces[BODIES] = {};
    acceleration accels[BODIES] = {};
    particle bbuff[BODIES] = {};
    particle rbuff[BODIES*BODIES] = {};
	
    boolean is_body;
	
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
    MPI_Bcast(particles, sizeof(BODIES), MPI_BYTE, 0, MPI_COMM_WORLD);

    for (t = 0; t < TIMESTEP; t++) {
        for (i = rank*(BODIES/WIDTH); i < (rank*(BODIES/WIDTH))+(BODIES/WIDTH); i++){
            fy, fx = 0;
            new_particles[i] = particles[i];
			bbuff[i] = particles[i];
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
			
			MPI_Allgather(new_particles, sizeof(BODIES), MPI_BYTE, rbuff, size*sizeof(BODIES), MPI_BYTE, MPI_COMM_WORLD);
			
            rbuff[i].yv += accels[i].y;
            rbuff[i].xv += accels[i].x;
            rbuff[i].y += rbuff[i].yv;
            rbuff[i].x += rbuff[i].xv;
        }
		
        for (i = 0; i < BODIES; i++) {
            particles[i] = rbuff[i];
        }
		if(rank == 0){
			sprintf(buffer, "para-%d.pbm", t);
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
    }
	if(rank == 0){
		gettimeofday(&tim2, NULL);
		double t2 = tim2.tv_sec+(tim2.tv_usec/100000.0);
		printf("                Parallel took: Total time taken in seconds: %f\n",t2-t1);
	}
	
MPI_Finalize();
return 0;
}
