# cblas 3.6.0 + clapack 3.2.1 (adapted to Myriad 2)

This library contains a combination of CBLAS and CLAPACK libraries for Myriad 2.

##### Depencences

None.

## cblas 3.6.0

This package contains C interface to Legacy BLAS, connecting to CLAPLACK library.

Information on the BLAS standards, including both the legacy and updated interface standards, is available online from the BLAS Homepage and BLAS Technical Forum web-site.

BLAS Homepage
http://www.netlib.org/blas/
BLAS Technical Forum
http://www.netlib.org/blas/blast-forum/

The following papers contain the specifications for Level 1, Level 2 and Level 3 BLAS.

- C. Lawson, R. Hanson, D. Kincaid, F. Krogh, “Basic Linear Algebra Subprograms for Fortran Usage”, ACM Transactions on Mathematical Software, Vol. 5 (1979), Pages 308–325.
- J.J. Dongarra, J. DuCroz, S. Hammarling, R. Hanson, “An Extended Set of Fortran Basic Linear Algebra Subprograms”, ACM Transactions on Mathematical Software, Vol. 14, No. 1 (1988), Pages 1–32.
- J.J. Dongarra, I. Duff, J. DuCroz, S. Hammarling, “A Set of Level 3 Basic Linear Algebra Subprograms”, ACM Transactions on Mathematical Software, Vol. 16 (1990), Pages 1–28. 

Postscript versions of the latter two papers are available from http://www.netlib.org/blas/. A CBLAS wrapper for Fortran BLAS libraries is available from the same location.

## clapack 3.2.1

This package contains the individual routines of the C version of LAPACK.

The CLAPACK library was built using a Fortran to C conversion utility called f2c.  The entire Fortran 77 LAPACK library is run through f2c to obtain C code, and then modified to improve readability.  CLAPACK's goal is to provide LAPACK for someone who does not have access to a Fortran compiler.

CLAPACK is a freely-available software package. It is available from http://www.netlib.org via anonymous ftp and the World Wide Web.

#### Credit for the authors:

LAPACK Users' Guide, Third Edition. 

- Anderson, E. and Bai, Z. and Bischof, C. and Blackford, S. and Demmel, J. and Dongarra, J. and Du Croz, J. and Greenbaum, A. and Hammarling, S. and McKenney, A. and Sorensen, D. LAPACK Users' Guide. Third Edition. Society for Industrial and Applied Mathematics. 1999. Philadelphia, PA. ISBN 0-89871-447-8 (paperback).

