/* Attempt to pack windows of a max size into given window, exit failure */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>

struct mbin {
	/* from -x -y -g flags */
	unsigned x, y, gaps;
};

struct input {
	/* from stdin */
	unsigned min_w, min_h, max_w, max_h;
	unsigned long wid;
};

struct output {
	unsigned x, y, w, h;
	unsigned long wid;
};

struct bins {
	/* temporary storage of available sub-bins */
	unsigned x, y, w, h;
};

size_t
create_rect(struct input r[]) {
	size_t length = 0;
	char line[50];
	for (unsigned i = 0; fgets(line, sizeof(line), stdin); ++i) {
		sscanf(line, "%d %d %d %d %lx", &r[i].min_w, &r[i].min_h, &r[i].max_w, &r[i].max_h, &r[i].wid);
		if ((r[i].min_w == 0 || r[i].min_h == 0 || r[i].max_w == 0 || r[i].max_h == 0 || r[i].wid == 0)) {
			continue;
		}
		r[i].min_w = r[i].max_w;
		r[i].min_h = r[i].max_h;
		length++;
	}
	return length;
}

void
sort_bins(struct bins b[], size_t *bin_count) {
	struct bins temp;
	for (unsigned i = 1; i < *bin_count; i++) {
		for (unsigned j = 0; j < *bin_count - i; j++) {
			if ((b[j + 1].w * b[j + 1].h) > (b[j].w * b[j].h)) {
				temp = b[j];
				b[j] = b[j + 1];
				b[j + 1] = temp;
			}
		}
	}
}

void
sort_input(struct input r[], const size_t length) {
	struct input temp;
	for (size_t i = 1; i < length; i++) {
		for (size_t j = 0; j < length - i; j++) {
			if ((r[j + 1].max_w * r[j + 1].min_h) > (r[j].max_w * r[j].min_h)) {
				temp = r[j];
				r[j] = r[j + 1];
				r[j + 1] = temp;
			}
		}
	}
}

void
create_bins(struct bins bin[], struct output out[], size_t i, size_t j, size_t *bin_count, struct mbin mb) {
	/* New bins based on subsection of old  */
	unsigned x, y, w, h;
	x = bin[j].x;
	y = bin[j].y;
	w = bin[j].w;
	h = bin[j].h;
	/* rect smaller, make two sub bins */
	if (out[i].h + mb.gaps < h && out[i].w + mb.gaps < w) {
		bin[*bin_count] = (struct bins){
			.x = x + out[i].w + mb.gaps,
			.y = y,
			.w = w - out[i].w - mb.gaps,
			.h = h
		};
		bin[j].y += (out[i].h + mb.gaps);
		bin[j].h -= (out[i].h - mb.gaps);
		*bin_count += 1;
	}
	/* rect same height */
	else if (out[i].h + mb.gaps < h) {
		bin[j].y += (out[i].h + mb.gaps);
		bin[j].h -= (out[i].h - mb.gaps);
	} 
	/* rect same width */
	else if (out[i].w + mb.gaps < w) {
		bin[j].x += (out[i].w + mb.gaps);
		bin[j].w -= (out[i].w - mb.gaps);
	} 
	/* rect fills space, lose a bin */
	else {
		bin[j].w = 0;
		bin[j].h = 0;
		*bin_count -= 1;
	}
}

void
save_rect(struct bins bin[], struct output out[], struct input r[], size_t i, size_t j, struct mbin mb) {
	/* Store rect x y w h wid */
	out[i] = (struct output){
		.x = bin[j].x + (mb.gaps / 2),
		.y = bin[j].y + (mb.gaps / 2),
		.w = r[i].min_w,
		.h = r[i].min_h,
		.wid = r[i].wid};
}

bool
pack_bin(struct output out[], struct input r[], const size_t length, unsigned *bin_width, unsigned *bin_height, struct mbin mb) {
	/* Main algorithm */
	struct bins bin[50];
	for (int i = 0; i < 50; i++) {
		bin[i] = (struct bins){
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0
		};
  	}
  	/* default bin */
  	bin[0] = (struct bins){
		.x = 0,
		.y = 0,
		.w = *bin_width + mb.gaps,
		.h = *bin_height + mb.gaps
	};

	size_t bin_count = 1;
	bool rect_fits;

	/* loop through each rect */
	for (size_t i = 0; i < length; i++) {
		rect_fits = false;
		/* loop through each bin */
		for (size_t j = 0; j < bin_count; j++) {
			/* rect fits in current bin */
			if (r[i].min_w + mb.gaps <= bin[j].w && r[i].min_h + mb.gaps <= bin[j].h) {
				rect_fits = true;
				save_rect(bin, out, r, i, j, mb);
				create_bins(bin, out, i, j, &bin_count, mb);
				sort_bins(bin, &bin_count);
				break;
			}
		}
		/* if rect does not fit all bin */
		if (rect_fits == false) {
			/* Grow main bin if possible */
			if (mb.x > *bin_width) {
				*bin_width += 2;
			}
			if (mb.y > *bin_height) {
				*bin_height += 2;
			}
			/* We hit the max size, exit */
			if ((*bin_height >= mb.y) && (*bin_width >= mb.x)) {
				exit(1);
			}
			return true;
		}
	}
	return false;
}

int
main(int argc, char *argv[]) {
	struct mbin mb;
	mb.gaps = 0;
	int c;
	while ((c = getopt(argc, argv, "hg:x:y:")) != -1) {
		switch (c) {
		/* read in, all vars unsigned int */
		case 'g':
			sscanf(optarg, "%u", &mb.gaps);
			break;
		case 'x':
			sscanf(optarg, "%u", &mb.x);
			break;
		case 'y':
			sscanf(optarg, "%u", &mb.y);
			break;
		case 'h':
			fputs("Usage: simplepack -x screen_width -y screen_height -g gaps\n",
			stderr);
			exit(0);	
		}
	}

	struct input r[50];
	struct output out[50];
	
	const size_t length = (create_rect(r));
	sort_input(r, length);

	// Set the initial bin width to our first window after sort
	unsigned bin_width = r[0].max_w;  
	unsigned bin_height = r[0].max_h;
	unsigned limitcheck = 0;

	while(pack_bin(out, r, length, &bin_width, &bin_height, mb)) {
		if (limitcheck == mb.y) {
			exit(1);
		}
		limitcheck++;
	}
	exit(0);	
}
