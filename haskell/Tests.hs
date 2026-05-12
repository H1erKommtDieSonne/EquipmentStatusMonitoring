module Tests
    ( runTests
    , allTests
    ) where

import DeviceModel
import HashTree
import HashTreeMonad

--Несколько тестовых устройств
device1 :: Device
device1 = Device "Pump-1" 3232235777 HighPriority (Healthy 1200)

device2 :: Device
device2 = Device "Sensor-1" 3232235778 LowPriority (Faulty "overheat")

device3 :: Device
device3 = Device "Reserve-1" 3232235779 NoPriority (Reserve 5000 300)

--Такое же значение адреса как у device1
--тест что дерево умеет хранить несколько устройств по одному ключу
deviceDuplicateAddress :: Device
deviceDuplicateAddress = Device "Pump-1-Copy" 3232235777 HighPriority (Faulty "duplicate address test")

--После вставки устройство должно находиться по адресу
testInsertFind :: Bool
testInsertFind =
    findByAddress (deviceAddress device1) tree == [device1]
    where
        tree = insertDevice device1 emptyTree

--Поиск отсутствующего адреса должен вернуть пустой список
testFindMissing :: Bool
testFindMissing =
    null (findByAddress 12345 tree)
    where
        tree = fromList [device1, device2, device3]

--После удаления устройство не должно находиться
testDelete :: Bool
testDelete =
    null (findByAddress (deviceAddress device2) treeAfterDelete)
    where
        tree = fromList [device1, device2, device3]
        treeAfterDelete = deleteByAddress (deviceAddress device2) tree

--fromList должен добавить все устройства
testFromList :: Bool
testFromList =
    treeSize tree == 3 &&
    findByAddress (deviceAddress device1) tree == [device1] &&
    findByAddress (deviceAddress device2) tree == [device2] &&
    findByAddress (deviceAddress device3) tree == [device3]
    where
        tree = fromList [device1, device2, device3]

--Если два устройства имеют один адрес, поиск должен вернуть оба
testDuplicateAddress :: Bool
testDuplicateAddress =
    length (findByAddress (deviceAddress device1) tree) == 2
    where
        tree = fromList [device1, deviceDuplicateAddress]

--Проверка монадной последовательности операций
--добавить два устройства, потом найти второе
testMonadAddFind :: Bool
testMonadAddFind =
    found == [device2] && treeSize finalTree == 2
    where
        action = do
            addDeviceM device1
            addDeviceM device2
            findByAddressM (deviceAddress device2)
        (found, finalTree) = runWithEmpty action

--Проверка монадного удаления
testMonadDelete :: Bool
testMonadDelete =
    null found && treeSize finalTree == 1
    where
        action = do
            addDeviceM device1
            addDeviceM device2
            deleteByAddressM (deviceAddress device1)
            findByAddressM (deviceAddress device1)
        (found, finalTree) = runWithEmpty action

--Список всех тестов
allTests :: [(String, Bool)]
allTests =
    [ ("insert/find", testInsertFind)
    , ("find missing", testFindMissing)
    , ("delete", testDelete)
    , ("fromList", testFromList)
    , ("duplicate address", testDuplicateAddress)
    , ("monad add/find", testMonadAddFind)
    , ("monad delete", testMonadDelete)
    ]

--Печать результата одного теста
printTestResult :: (String, Bool) -> IO ()
printTestResult (name, result) =
    if result
        then putStrLn ("[OK]   " ++ name)
        else putStrLn ("[FAIL] " ++ name)

--Запуск всех тестов
runTests :: IO ()
runTests = do
    putStrLn "Tests:"
    mapM_ printTestResult allTests
