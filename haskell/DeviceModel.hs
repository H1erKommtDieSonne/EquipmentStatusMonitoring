--В исходном C++-проекте есть несколько классов устройств
--В Haskell вместо наследования удобнее сделать один тип Device
--и отдельный тип DeviceStatus, который хранит состояние устройства.


module DeviceModel
    ( Address
    , ServicePriority(..)
    , DeviceStatus(..)
    , Device(..)
    , addressBytes
    , addressToDotted
    , showPriority
    , showStatus
    , requiresService
    , isFaultyDevice
    ) where

--адрес устройства хранится как 32-битное беззнаковое число
--используем Int, потому что так проще читать
--адрес из текстового файла и работать с ним без дополнительных библиотек
type Address = Int

--Приоритет обслуживания устройства
--Порядок конструкторов выбран от меньшего приоритета к большему
data ServicePriority
    = NoPriority
    | LowPriority
    | HighPriority
    deriving (Show, Eq, Ord)

--Состояние устройства
--Healthy хранит время работы без неисправностей
--Faulty хранит текстовое описание неисправности
--Reserve хранит время работы и время ожидания ввода в строй
data DeviceStatus
    = Healthy Int
    | Faulty String
    | Reserve Int Int
    deriving (Show, Eq)

--Основной тип устройства
--Это аналог общего набора полей из C++-классов проекта
--имя, сетевой адрес, приоритет обслуживания и состояние
data Device = Device
    { deviceName     :: String
    , deviceAddress  :: Address
    , devicePriority :: ServicePriority
    , deviceStatus   :: DeviceStatus
    } deriving (Show, Eq)

--Разбиваем 32-битный адрес на 4 байта
--эти байты используются в дереве хэшей как путь от корня к листу.
-- Пример 3232235777 = 192.168.1.1
-- addressBytes 3232235777 = [192,168,1,1]
addressBytes :: Address -> [Int]
addressBytes addr =
    [ (addr `div` 16777216) `mod` 256
    , (addr `div` 65536)    `mod` 256
    , (addr `div` 256)      `mod` 256
    , addr                  `mod` 256
    ]

--Соединяем список строк через разделитель
joinWith :: String -> [String] -> String
joinWith _ [] = ""
joinWith _ [x] = x
joinWith sep (x:xs) = x ++ sep ++ joinWith sep xs

--Преобразуем числовой адрес в привычную форму IPv4
addressToDotted :: Address -> String
addressToDotted addr = joinWith "." (map show (addressBytes addr))

--вывод приоритета
showPriority :: ServicePriority -> String
showPriority NoPriority   = "нет"
showPriority LowPriority  = "низкий"
showPriority HighPriority = "высокий"

--вывод состояния устройства
showStatus :: DeviceStatus -> String
showStatus (Healthy uptime) =
    "исправно, время работы = " ++ show uptime ++ " сек."
showStatus (Faulty description) =
    "неисправно, причина = " ++ description
showStatus (Reserve uptime waitTime) =
    "резерв, время работы = " ++ show uptime ++
    " сек., ожидание ввода = " ++ show waitTime ++ " сек."


--обслуживание требуется, если приоритет не отсутствует
requiresService :: Device -> Bool
requiresService device = devicePriority device /= NoPriority

--Является ли устройство неисправным
isFaultyDevice :: Device -> Bool
isFaultyDevice device =
    case deviceStatus device of
        Faulty _ -> True
        _        -> False
