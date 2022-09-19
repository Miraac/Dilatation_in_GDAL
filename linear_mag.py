input('program wymaga bibliotek pandas oraz sklearn. jesli biblioteki wystepuja kliknij enter by kontynuowac')
import pandas as pd
input('jest pd')
from sklearn.linear_model import LinearRegression
input('jest sklearn')
#plik
mag = pd.read_csv('cloudmetricsmh505.csv')
mag.head()

#przygotowanie danych - usuwanie zbednych kolumn
mag2=mag.drop(mag.iloc[:,0:2],axis=1)
mag2.drop(mag2.iloc[:,9:13],axis=1,inplace=True)
mag2.drop(mag2.iloc[:,46:85],axis=1,inplace=True)
print('zaimportowano plik glowny(dane uczace)')
#tworzenie danych i probki
inp = input('podaj pelna sciezke pliku z danymi testowymi.(dodaj nazwe rozszerzenia do nazwy pliku: .csv: \n')
try:
    test = pd.read_csv(inp)
except FileNotFoundError:
    print('bledna nazwa pliku')
    input('wcisnij enter by zakonczyc dzialanie programu')
    exit()
print('pozyskano dane')
input('wcisnij enter aby kontynuowac')
test=test.drop(mag.iloc[:,0:2],axis=1)
test.drop(test.iloc[:,9:13],axis=1,inplace=True)
test.drop(test.iloc[:,46:],axis=1,inplace=True)
#test = mag.iloc[[14,30,37,46,67,73,84,92,110,112]]
#mag2=mag2.drop([14,30,37,46,67,73,84,92,110,112])
cat = {'lesny':1,'porolny':2}

y=mag2['rodzaj']
X = mag2
del X['rodzaj']

#testy=test['rodzaj']
#del test['rodzaj']

#wprowadzanie danych numerycznych
y = y.apply(lambda x: cat[x])


#tworzenie modelu

lr = LinearRegression()
lr.fit(X,y)
print('dokladnosc modelu wynosi: ',lr.score(X,y)*100,'%')

spp = lr.predict(test)
print('liczbowe wyniki klasyfikacji: ',spp)
input('wcisnij enter by kontynuowac')
print('wyniki koncowe:')
for f,s in zip(test,spp):
    if round(s) == 1:
        print('lesny')
    elif round(s) == 2:
        print('porolny')
    else:
        print('brak')
input('\nwcisnij enter aby wyjsc')


