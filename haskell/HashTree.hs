--Дерево хэшей по сетевому адресу устройства
--1. Берём 32-битный адрес устройства
--2. Разбиваем его на 4 байта [b1, b2, b3, b4]
--3. Каждый байт выбирает ветку на соответствующем уровне дерева
--4. В листе хранится список устройств с данным адресом
--Такая структура нужна как индекс по адресу устройства
--В исходном проекте DeviceCollection ищет устройство линейно
--по vector, а дерево хэшей показывает альтернативный способ хранения


module HashTree
    ( HashTree(..)
    , emptyTree
    , insertDevice
    , findByAddress
    , deleteByAddress
    , fromList
    , toList
    , treeSize
    ) where

import DeviceModel

--Узел дерева хранит номер уровня и список пар
--значение байта и поддерево
--
--Лист хранит пары
--адрес и список устройств с этим адресом
--Список в листе нужен на случай совпадений ключей или если в данных
--случайно окажется несколько устройств с одним и тем же адресом
data HashTree
    = Empty
    | HashNode Int [(Int, HashTree)]
    | HashLeaf [(Address, [Device])]
    deriving (Show, Eq)

--Пустое дерево
emptyTree :: HashTree
emptyTree = Empty

--Добавляет устройство в дерево по его сетевому адресу
insertDevice :: Device -> HashTree -> HashTree
insertDevice device tree =
    insertByBytes (deviceAddress device) (addressBytes (deviceAddress device)) device tree

--Рекурсивная часть вставки
--Второй аргумент это ещё не обработанные байты адреса
insertByBytes :: Address -> [Int] -> Device -> HashTree -> HashTree
insertByBytes addr [] device Empty =
    HashLeaf [(addr, [device])]

insertByBytes addr [] device (HashLeaf bucket) =
    HashLeaf (insertIntoBucket addr device bucket)


--если байты закончились, но мы попали не в лист
--Оставляем поведение безопасным т е  создаём лист для нового адреса
insertByBytes addr [] device (HashNode _ _) =
    HashLeaf [(addr, [device])]

insertByBytes addr (byte:restBytes) device Empty =
    HashNode currentLevel [(byte, insertByBytes addr restBytes device Empty)]
    where
--Уровень нужен только для понятного вывода
        currentLevel = 4 - length (byte:restBytes)

insertByBytes addr (byte:restBytes) device (HashNode level children) =
    HashNode level (insertChild byte newChild children)
    where
        oldChild = findChild byte children
        newChild = insertByBytes addr restBytes device oldChild

--Если дерево построено правильно, такого случая не будет
--лист должен появляться только после обработки всех 4 байтов
--Но для безопасности просто добавляем устройство в лист
insertByBytes addr _ device (HashLeaf bucket) =
    HashLeaf (insertIntoBucket addr device bucket)

--Добавляет устройство в список элементов листа
insertIntoBucket :: Address -> Device -> [(Address, [Device])] -> [(Address, [Device])]
insertIntoBucket addr device [] =
    [(addr, [device])]
insertIntoBucket addr device ((oldAddr, devices):rest)
    | addr == oldAddr = (oldAddr, device : devices) : rest
    | otherwise       = (oldAddr, devices) : insertIntoBucket addr device rest

--Ищет устройство по адресу
--Возвращает список, потому что теоретически несколько устройств
--могут иметь один и тот же адрс
findByAddress :: Address -> HashTree -> [Device]
findByAddress addr tree =
    findByBytes addr (addressBytes addr) tree

findByBytes :: Address -> [Int] -> HashTree -> [Device]
findByBytes _ _ Empty = []
findByBytes addr [] (HashLeaf bucket) = findInBucket addr bucket
findByBytes _ [] (HashNode _ _) = []
findByBytes addr (byte:restBytes) (HashNode _ children) =
    findByBytes addr restBytes (findChild byte children)
findByBytes addr _ (HashLeaf bucket) = findInBucket addr bucket

--Ищет адрес внутри листа
findInBucket :: Address -> [(Address, [Device])] -> [Device]
findInBucket _ [] = []
findInBucket addr ((oldAddr, devices):rest)
    | addr == oldAddr = reverse devices
    | otherwise       = findInBucket addr rest

--Удаляет все устройства с заданным адресом
deleteByAddress :: Address -> HashTree -> HashTree
deleteByAddress addr tree =
    deleteByBytes addr (addressBytes addr) tree

deleteByBytes :: Address -> [Int] -> HashTree -> HashTree
deleteByBytes _ _ Empty = Empty

deleteByBytes addr [] (HashLeaf bucket) =
    case deleteFromBucket addr bucket of
        []        -> Empty
        newBucket -> HashLeaf newBucket

deleteByBytes _ [] node@(HashNode _ _) = node

deleteByBytes addr (byte:restBytes) (HashNode level children) =
    case deleteChild addr byte restBytes children of
        []          -> Empty
        newChildren -> HashNode level newChildren

deleteByBytes addr _ (HashLeaf bucket) =
    case deleteFromBucket addr bucket of
        []        -> Empty
        newBucket -> HashLeaf newBucket

--Удаляет адрес из листп=а
deleteFromBucket :: Address -> [(Address, [Device])] -> [(Address, [Device])]
deleteFromBucket _ [] = []
deleteFromBucket addr ((oldAddr, devices):rest)
    | addr == oldAddr = rest
    | otherwise       = (oldAddr, devices) : deleteFromBucket addr rest

--Ищет поддерево по значению байта
--Если такой ветки нет возвращает Empty
findChild :: Int -> [(Int, HashTree)] -> HashTree
findChild _ [] = Empty
findChild byte ((oldByte, child):rest)
    | byte == oldByte = child
    | otherwise       = findChild byte rest

--Вставляет или заменяет поддерево для заданного байта
insertChild :: Int -> HashTree -> [(Int, HashTree)] -> [(Int, HashTree)]
insertChild byte child [] = [(byte, child)]
insertChild byte child ((oldByte, oldChild):rest)
    | byte == oldByte = (oldByte, child) : rest
    | otherwise       = (oldByte, oldChild) : insertChild byte child rest

--Удаляет адрес из нужного дочернего поддерева
--Если после удаления поддерево стало пустым, сама ветка тоже убирается
deleteChild :: Address -> Int -> [Int] -> [(Int, HashTree)] -> [(Int, HashTree)]
deleteChild _ _ _ [] = []
deleteChild addr byte restBytes ((oldByte, child):rest)
    | byte == oldByte =
        case deleteByBytes addr restBytes child of
            Empty    -> rest
            newChild -> (oldByte, newChild) : rest
    | otherwise = (oldByte, child) : deleteChild addr byte restBytes rest

--Строит дерево из списка устройств
fromList :: [Device] -> HashTree
fromList devices = foldl addOne emptyTree devices
    where
        addOne tree device = insertDevice device tree

--Возвращает все устройства из дерева обычным списком
toList :: HashTree -> [Device]
toList Empty = []
toList (HashLeaf bucket) = concatMap devicesFromPair bucket
    where
        devicesFromPair (_, devices) = reverse devices
toList (HashNode _ children) = concatMap devicesFromChild children
    where
        devicesFromChild (_, child) = toList child

--Количество устройств в дереве
treeSize :: HashTree -> Int
treeSize tree = length (toList tree)
