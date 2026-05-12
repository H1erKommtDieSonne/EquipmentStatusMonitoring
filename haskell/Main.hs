module Main where

import DeviceModel
import DeviceParser
import HashTree
import HashTreeMonad
import Tests

--Адрес который будем искать в демонстрации
--Это 192.168.1.1 в числовой форме
demoSearchAddress :: Address
demoSearchAddress = 3232235777

--Новое устройство которое добавим уже после чтения файла
newDevice :: Device
newDevice = Device "Controller-New" 3232235790 HighPriority (Healthy 100)

main :: IO ()
main = do
    putStrLn "Дерево хэшей по сетевому адресу устройства"
    putStrLn ""
--Читаем файл с тестовыми устройствами
    contents <- readFile "devices.txt"

--parseDevices не работает с IO напрямую
--а только преобразует строку в список устройств
    let devices = parseDevices contents

    putStrLn "Загруженные устройства:"
    mapM_ (putStrLn . showDevice) devices
    putStrLn ""

--Строим дерево из списка устройств
    let tree = fromList devices

    putStrLn ("Количество устройств в дереве: " ++ show (treeSize tree))
    putStrLn ""

--Демонстрация поиска
    putStrLn ("Поиск по адресу " ++ show demoSearchAddress ++
              " (" ++ addressToDotted demoSearchAddress ++ "):")
    printSearchResult (findByAddress demoSearchAddress tree)
    putStrLn ""

--Демонстрация монадного добавления
--Состояние дерева передаётся внутри TreeState
    let action = do
            addDeviceM newDevice
            findByAddressM (deviceAddress newDevice)

    let (foundNewDevice, treeAfterAdd) = runTreeState action tree

    putStrLn "После монадного добавления нового устройства:"
    printSearchResult foundNewDevice
    putStrLn ("Количество устройств стало: " ++ show (treeSize treeAfterAdd))
    putStrLn ""

--тесты
    runTests

--результат поиска
printSearchResult :: [Device] -> IO ()
printSearchResult [] = putStrLn "Устройства с таким адресом не найдены"
printSearchResult devices = mapM_ (putStrLn . showDevice) devices
