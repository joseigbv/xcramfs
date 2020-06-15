
/*****************************
 * cramfs file extract
 * compile: gcc -m32 -Wall -ILZMA_C xcramfs.c LZMA_C/decode.o LZMA_C/LzmaDecode.o -o xcramfs
 *****************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "cramfs.h"

// cifrado lzma (7zip)
#include "LzmaDecode.h"
#include "decode.h"

// configuracion
#define FNAME "data.cramfs"
#define SZ_PATH 1024
#define SZ_BLK 65536
#define SZ_NAME 64
#define BASE "x"

//  macros ayuda
#define ABORT(msg) { perror(msg); exit(-1); }
#define TRY(x, msg) if ((x) == -1) ABORT(msg)
#define SZ(x) sizeof(x)


// debug
void print_inode(struct cramfs_inode inode)
{
	// mostramos
	printf( "struct inode:\n" 
		"=============\n\n"
		"\tmode / uid:\t\t0x%04x / 0x%04x\n"
		"\tsize / gid:\t\t0x%06x / 0x%02x\n"
		"\tnamelen / offset:\t0x%02x / 0x%07x\n"
		"\n",
		inode.mode, inode.uid,
		inode.size, inode.gid, 
		inode.namelen, inode.offset
	);
}


// lectura dir, recursivo
void read_dir(int fs, char base[], u32 offset, int size)
{
	int r, namelen, off, sz = 0;
	int fo, nblocks, pos, len, i;
	struct cramfs_inode inode;
	char name[SZ_NAME], path[SZ_PATH];
	u32 *ptrs;
	static u8 in[SZ_BLK];
	static u8 out[SZ_BLK];
	char link[SZ_PATH];

	// creamos directorio de salida
	mkdir(BASE, 0755);

	// posicionamos en fs
	TRY(lseek(fs, offset, SEEK_SET), "lseek");

	// para cada entrada del dir...
	do 
	{
		// leemos siguiente inode
		TRY(r = read(fs, &inode, SZ(inode)), "read inode");

		// control de errores
		if (r < SZ(inode)) ABORT("read inode");

		// bytes leidos
		sz += SZ(inode);

		// leemos nombre fichero, dir, sym
		namelen = inode.namelen * 4;
		TRY(r = read(fs, name, namelen), "read filename");
		name[r] = 0; 

		// act bytes leidos
		sz += namelen;

		// control de errores
		if (r < namelen) ABORT("read filename");

		// is it a regular file?
		if (S_ISREG(inode.mode) || S_ISLNK(inode.mode))
		{
			// mostramos fichero
			sprintf(path, "%s/%s", base, name);
			printf("%s (%s)\n", path, S_ISREG(inode.mode) ? "file" : "link");

			// guardamos pos en fs
			TRY(off = lseek(fs, 0, SEEK_CUR), "lseek");

			// numero bloques a leer ?
			nblocks = (inode.size - 1) / SZ_BLK + 1;

			// reservamos memoria para punteros de bloque
			if (! (ptrs = malloc(nblocks * 4))) ABORT("malloc");
	
			// recolocamos pos en fs
			TRY(lseek(fs, inode.offset * 4, SEEK_SET), "lseek");

			// leemos punteros de bloque
			TRY(r = read(fs, ptrs, nblocks * 4), "read pblocks");
			
			// control de errores
			if (r < nblocks * 4) ABORT("read pblocks");

			// donde empiezan datos
			pos = r + (inode.offset * 4);

			// creamos el fichero
			TRY(fo = creat(path, inode.mode), "creat");
			TRY(chown(path, inode.uid, inode.gid), "chown");

			// leemos cada bloque comprimido
			for (i = 0; i < nblocks; i++)
			{
				// tamanio de bloque?
				len = ptrs[i] - pos;

				// leemos bloque
				TRY(r = read(fs, in, len), "read block");

				// control de errores
				if (r < len) ABORT("read block");

				// descomprimimos
      				TRY(r = lzma_decode(out, SZ_BLK, in, len), "decomnp");
	
				// guardamos fichero
				TRY(write(fo, out, r), "write block");

				// actualizamos pos 
				pos += len;
			}

			// cerramos fichero
			close(fo);

			// es un link? sustitimos por link
			if (S_ISLNK(inode.mode))
			{
				// borramos fichero
				unlink(path);

				// y creamos link
				out[r] = 0;
				snprintf(link, SZ_PATH, "%s%s", BASE, out);
				TRY(symlink(link, path), "symlink");
			}

			// liberamos memoria
			free(ptrs);

			// restauramos puntero
			TRY(lseek(fs, off, SEEK_SET), "lseek");
		}

		// directory?
		else if (S_ISDIR(inode.mode))
		{
			// muestra nombre dir
			sprintf(path, "%s/%s", base, name);
			printf("%s (dir)\n", path);

			// creamos el directorio
			TRY(mkdir(path, inode.mode), "mkdir");
			TRY(chown(path, inode.uid, inode.gid), "chown");

			// siguiente dir 
			TRY(off = lseek(fs, 0, SEEK_CUR), "lseek");
			read_dir(fs, path, inode.offset * 4, inode.size);
			TRY(lseek(fs, off, SEEK_SET), "lseek");
		}

		// otro o error?
		else ABORT("mode");

	} while (sz < size);
}

// principal
int main()
{
	int fs, r;
	struct cramfs_super super;

	printf("\n*** cramfs test ***\n\n");

	// abrimos fichero
	TRY(fs = open(FNAME, 0), "open fs");

	// leemos superbloque
	TRY(r = read(fs, &super, SZ(super)), "read super");

	// control errores
	if (r < SZ(super)) ABORT("read super");
	if (super.magic != CRAMFS_MAGIC) ABORT("no valid fs");

	// leemos nodo raiz
	read_dir(fs, BASE, super.root.offset * 4, super.root.size);

	// cerramos fichero cramfs
	close(fs);

	return 0;
}
