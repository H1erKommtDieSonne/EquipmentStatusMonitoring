--Чтение устройств из простого текстового формата
--Формат строк в файле devices.txt

module DeviceParser
    ( parsePriority
    , parseDevice
    , parseDevices
    , showDevice
    ) where

import Data.Char (isSpace, toLower)
import DeviceModel

--Удаляет пробелы в начале и в конце строки
trim :: String -> String
trim = reverse . dropWhile isSpace . reverse . dropWhile isSpace

--Переводит строку к нижнему регистру
lowerString :: String -> String
lowerString = map toLower . trim

--Разделение строки по символу разделителю
splitBy :: Char -> String -> [String]
splitBy _ [] = [""]
splitBy delimiter (x:xs)
    | x == delimiter = "" : rest
    | otherwise      = (x : head rest) : tail rest
    where
        rest = splitBy delimiter xs

--чтение Int
readIntMaybe :: String -> Maybe Int
readIntMaybe text =
    case reads (trim text) of
        [(value, rest)] | all isSpace rest -> Just value
        _                                  -> Nothing

--Разбор приоритета
parsePriority :: String -> Maybe ServicePriority
parsePriority text =
    case lowerString text of
        "no"       -> Just NoPriority
        "none"     -> Just NoPriority
        "nopriority" -> Just NoPriority
        "нет"      -> Just NoPriority
        "low"      -> Just LowPriority
        "низкий"   -> Just LowPriority
        "high"     -> Just HighPriority
        "высокий"  -> Just HighPriority
        _          -> Nothing

--Разбор одной строки с устройством
parseDevice :: String -> Maybe Device
parseDevice line =
    case map trim (splitBy ';' line) of
        [status, name, addrText, priorityText, uptimeText]
            | lowerString status == "healthy" ->
                makeHealthy name addrText priorityText uptimeText

        [status, name, addrText, priorityText, faultText]
            | lowerString status == "faulty" ->
                makeFaulty name addrText priorityText faultText

        [status, name, addrText, priorityText, uptimeText, waitText]
            | lowerString status == "reserve" ->
                makeReserve name addrText priorityText uptimeText waitText

        _ -> Nothing

--Создание исправного устройства из текстовых полей
makeHealthy :: String -> String -> String -> String -> Maybe Device
makeHealthy name addrText priorityText uptimeText =
    case (readIntMaybe addrText, parsePriority priorityText, readIntMaybe uptimeText) of
        (Just addr, Just priority, Just uptime) ->
            Just (Device name addr priority (Healthy uptime))
        _ -> Nothing

--Создание неисправного устройства из текстовых полей
makeFaulty :: String -> String -> String -> String -> Maybe Device
makeFaulty name addrText priorityText faultText =
    case (readIntMaybe addrText, parsePriority priorityText) of
        (Just addr, Just priority) ->
            Just (Device name addr priority (Faulty faultText))
        _ -> Nothing

--Создание резервного устройства из текстовых полей
makeReserve :: String -> String -> String -> String -> String -> Maybe Device
makeReserve name addrText priorityText uptimeText waitText =
    case ( readIntMaybe addrText
         , parsePriority priorityText
         , readIntMaybe uptimeText
         , readIntMaybe waitText
         ) of
        (Just addr, Just priority, Just uptime, Just waitTime) ->
            Just (Device name addr priority (Reserve uptime waitTime))
        _ -> Nothing

--Убираем пустые строки и комментарии.
--Комментарием считается строка, начинающаяся с #
prepareLines :: String -> [String]
prepareLines text = filter isUsefulLine (map trim (lines text))
    where
        isUsefulLine [] = False
        isUsefulLine ('#':_) = False
        isUsefulLine _ = True

--Собирает только успешно распознанные устройства
collectParsed :: [Maybe Device] -> [Device]
collectParsed [] = []
collectParsed (Just device : rest) = device : collectParsed rest
collectParsed (Nothing : rest) = collectParsed rest

--Разбор всего файла
parseDevices :: String -> [Device]
parseDevices text = collectParsed (map parseDevice (prepareLines text))

--вывод устройства
showDevice :: Device -> String
showDevice device =
    deviceName device ++
    " | address = " ++ show (deviceAddress device) ++
    " (" ++ addressToDotted (deviceAddress device) ++ ")" ++
    " | priority = " ++ showPriority (devicePriority device) ++
    " | status = " ++ showStatus (deviceStatus device)
