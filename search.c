#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "cramfs.h"

// fichero a leer
#define FNAME "openrg.rta9211w_6_0_18_1_11.img"

// patron a buscar 
// // 0x28, 0xcd, 0x3d, 0x45
char PAT[] = { 0x45, 0x3d, 0xcd, 0x28 };


// macros ayuda
#define ABORT(msg) { perror(msg); return(-1); }
#define TRY(x, msg) if ((x) == -1) ABORT(msg)
#define SZ(x) sizeof(x)
#define HS(x) super.signature[x]
#define HF(x) super.fsid[x]
#define HN(x) super.fsid[x]


// funcion principal
int main()
{
	int f, pos = 0, i, r;
	int len = sizeof(PAT);
	struct cramfs_super super;
	char c;

	printf("Searching for CRAMFS filesystem...\n");

	// abrimos fichero
	TRY(f = open(FNAME, 0), "open");

	// leemos 1 caracter
	while (read(f, &c, 1))
	{
		// leemos resto de caracteres
		for (i = 0; (i < len) && (c == PAT[i]); i++)
			if (! read(f, &c, 1)) break;

		// encontrado patron? 
		if (i == len) 
		{
			printf("* Pattern found in byte %d.\n\n", pos);

			// retrocedemos 
			TRY(lseek(f, pos, SEEK_SET), "lseek");

			// leemos superbloque
			TRY(r = read(f, &super, SZ(super)), "read super");

			// control errores
			if (r < SZ(super)) ABORT("read super");

			// mostramos estructura super
			printf( "Struct Super:\n"
				"=============\n\n"
				"Magic:\t\t0x%08x\n"
				"Size:\t\t0x%08x\n"
				"Flags:\t\t0x%08x\n"
				"Future:\t\t0x%08x\n"
				"Signature:\t%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\n"
				"Fsid:\t\t0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
				"Name:\t\t0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
				"\n\n",
				super.magic,
				super.size,
				super.flags,
				super.future,
				HS(0), HS(1), HS(2), HS(3), HS(4), HS(5), HS(6), HS(7), HS(8), HS(9), HS(10), HS(11), HS(12), HS(13), HS(14), HS(15),
				HF(0), HF(1), HF(2), HF(3), HF(4), HF(5), HF(6), HF(7), HF(8), HF(9), HF(10), HF(11), HF(12), HF(13), HF(14), HF(15),
				HN(0), HN(1), HN(2), HN(3), HN(4), HN(5), HN(6), HN(7), HN(8), HN(9), HN(10), HN(11), HN(12), HN(13), HN(14), HN(15)
			);
		}

		// actualizamos pos
		pos = pos + i + 1;
	}

	// cerramos fichero
	close(f);

	return 0;
}
