#include <stdio.h>

#include <gdal/gdal.h>
#include <gdal/cpl_conv.h>

#define INFILE "input.tif"
#define OUTFILE "output.tif"


int main()
{
  // zmienne do obsługi struktur GDAL
  GDALDatasetH src_dataset_h, dst_dataset_h;
  GDALRasterBandH src_band_h, dst_band_h;
  GDALDriverH driver;
  CPLErr res;

  // bufory
  double *buffer_out, *buffer1, *buffer2, *buffer3, *pom;
  // macierz tranformacji afinicznej
  double geo_transform[6];
  // wiersz i kolumny
  int cols, rows, row, col;
  // odwzorowanie
  const char *wkt;
  // obsługa nodata
  int is_nodata = 0;
  double nodata = 0.0;

  // inicjowanie sterowników biblioteki GDAL
  GDALAllRegister();

  // otwieranie pliku wejściowego
  src_dataset_h = GDALOpen(INFILE, GA_ReadOnly );
  if( src_dataset_h == NULL ) {
    printf("Nie mogę otworzyć pliku wejściowego!\n");
    exit(1);
  }
  
  // pobieranie informacji o rozmiarze, odwzorowaniu, transformacji oraz nodata
  src_band_h = GDALGetRasterBand(src_dataset_h, 1);
  cols = GDALGetRasterBandXSize(src_band_h);
  rows = GDALGetRasterBandYSize(src_band_h);
  wkt  = GDALGetProjectionRef(src_dataset_h);
  GDALGetGeoTransform(src_dataset_h, geo_transform);
  nodata = GDALGetRasterNoDataValue(src_band_h, &is_nodata);

  printf("Rozmiar pliku wejściowego: %d x %d komórek\n",cols,rows);

  // tworzenie pliku wyjściowego
  driver = GDALGetDriverByName("GTiff");
  if(driver==NULL) {
    printf("Problem with GeoTiff driver!\n");
    exit(1);
  }

  dst_dataset_h = GDALCreate(driver,OUTFILE,cols,rows,1,GDT_Float64,NULL);

  if(!dst_dataset_h) {
    printf("Problem with creating file!\n");
    exit(1);
  }

  // ustawianie odwzorowania
  GDALSetProjection(dst_dataset_h, wkt);
  // ustawianie transformacji
  GDALSetGeoTransform(dst_dataset_h, geo_transform);

  // tworzenie pierwszej warstwy
  dst_band_h = GDALGetRasterBand(dst_dataset_h, 1);
  GDALSetRasterNoDataValue(dst_band_h, nodata);

  // allokowanie buforów
  buffer1 = (double *) CPLMalloc(cols*sizeof(double));
  buffer2 = (double *) CPLMalloc(cols*sizeof(double));
  buffer3 = (double *) CPLMalloc(cols*sizeof(double));
  buffer_out = (double *) CPLMalloc(cols*sizeof(double));

  //Wypełnianie bufora wyjściowego wartościami nodata
  for(col=0; col<cols; col++)
    buffer_out[col] = nodata;

  //pierwszy wiersz
  res = GDALRasterIO(src_band_h, GF_Read, 0, 0, cols, 1,
                   buffer1, cols, 1, GDT_Float64, 0, 0);
  if(res>CE_Warning) {
      printf("Błąd odczytu z pliku!\n");
      exit(1);
    }
  // drugi wiersz
  res = GDALRasterIO(src_band_h, GF_Read, 0, 1, cols, 1,
                   buffer2, cols, 1, GDT_Float64, 0, 0);
  if(res>CE_Warning) {
      printf("Błąd odczytu z pliku!\n");
      exit(1);
    }

  //zapis putego wiersza na samej górze rastra
  res = GDALRasterIO(dst_band_h, GF_Write, 0, 0, cols, 1,
                   buffer_out, cols, 1, GDT_Float64, 0, 0);
  if(res>CE_Warning) {
      printf("Błąd zapisu do pliku!\n");
      exit(1);
    }


  //
  for(row=2; row<rows; row++) {

    res = GDALRasterIO(src_band_h, GF_Read, 0, row, cols, 1,
                   buffer3, cols, 1, GDT_Float64, 0, 0);
    if(res>CE_Warning) {
      printf("Błąd odczytu z pliku!\n");
      exit(1);
    }

    // dylatacja

    for(col=1;col<cols-1;col++){
      if(buffer1[col-1]||buffer1[col]||buffer1[col+1]||buffer2[col-1]||buffer2[col+1]||buffer3[col-1]||buffer3[col]||buffer3[col+1]>=buffer2[col]){
        int m = buffer1[col-1];
        if(buffer1[col]>m) {m=buffer1[col];}
        if(buffer1[col+1]>m) {m=buffer1[col+1];}
        if(buffer2[col-1]>m) {m=buffer2[col-1];}
        if(buffer2[col+1]>m) {m=buffer2[col+1];}
        if(buffer3[col-1]>m) {m=buffer3[col-1];}
        if(buffer3[col]>m) {m=buffer3[col];}
        if(buffer3[col+1]>m) {m=buffer3[col+1];}
        buffer_out[col] = m;
        }
      if(buffer2[col]==nodata){buffer_out[col]=nodata;}
      //else {buffer_out[col] = buffer2[col];}
        }


////////////////////////////////////////////////////

    res = GDALRasterIO(dst_band_h, GF_Write, 0, row-1, cols, 1,
                   buffer_out, cols, 1, GDT_Float64, 0, 0);
    if(res>CE_Warning) {
      printf("Błąd zapisu do pliku!\n");
      exit(1);
    }

    // rotacja buforów
    pom = buffer1; buffer1 = buffer2; buffer2 = buffer3; buffer3 = pom;
  }

  //Wypełnianie bufora wyjściowego wartościami nodata
  for(col=0; col<cols; col++)
    buffer_out[col] = nodata;
  //zapis putego wiersza na samym dole rastra
  res = GDALRasterIO(dst_band_h, GF_Write, 0, rows-1, cols, 1,
                   buffer_out, cols, 1, GDT_Float64, 0, 0);
  if(res>CE_Warning) {
      printf("Błąd zapisu do pliku!\n");
      exit(1);
    }

  // zwalnianie pamięci
  CPLFree(buffer_out);
  CPLFree(buffer1);
  CPLFree(buffer2);
  CPLFree(buffer3);
  // zamykanie
  GDALClose(src_dataset_h);
  GDALClose(dst_dataset_h);
}
