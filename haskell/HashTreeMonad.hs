--Монадная оболочка над HashTree
--TreeState это действие, которое получает старое дерево
--и возвращает результат типа a вместе с новым деревом

module HashTreeMonad
    ( TreeState(..)
    , runWithEmpty
    , addDeviceM
    , findByAddressM
    , deleteByAddressM
    , getTreeM
    , putTreeM
    ) where

import DeviceModel
import HashTree

--Действие над деревом
--Вход старое дерево
--Выход результат и новое дерево
newtype TreeState a = TreeState
    { runTreeState :: HashTree -> (a, HashTree) }

--Functor позволяет применить чистую функцию к результату действия
instance Functor TreeState where
    fmap f action = TreeState $ \tree ->
        let (value, newTree) = runTreeState action tree
        in (f value, newTree)

--Monad в современном хаскель требует наличие Applicative
instance Applicative TreeState where
    pure value = TreeState $ \tree -> (value, tree)

    funcAction <*> valueAction = TreeState $ \tree ->
        let (func, treeAfterFunc) = runTreeState funcAction tree
            (value, treeAfterValue) = runTreeState valueAction treeAfterFunc
        in (func value, treeAfterValue)

--Monad позволяет записывать последовательность операций через do
--Каждая следующая операция получает дерево изменённое предыдущей
instance Monad TreeState where
    action >>= nextAction = TreeState $ \tree ->
        let (value, newTree) = runTreeState action tree
        in runTreeState (nextAction value) newTree

--Запустить монадное действие на пустом дереве
runWithEmpty :: TreeState a -> (a, HashTree)
runWithEmpty action = runTreeState action emptyTree

--Монадная операция добавления устройства
addDeviceM :: Device -> TreeState ()
addDeviceM device = TreeState $ \tree ->
    ((), insertDevice device tree)

--Монадная операция поиска
--Дерево при поиске не меняется
findByAddressM :: Address -> TreeState [Device]
findByAddressM addr = TreeState $ \tree ->
    (findByAddress addr tree, tree)

--Монадная операция удаления
deleteByAddressM :: Address -> TreeState ()
deleteByAddressM addr = TreeState $ \tree ->
    ((), deleteByAddress addr tree)

--Получить текущее дерево из контекста
getTreeM :: TreeState HashTree
getTreeM = TreeState $ \tree -> (tree, tree)

--Заменить текущее дерево в контексте
putTreeM :: HashTree -> TreeState ()
putTreeM newTree = TreeState $ \_ -> ((), newTree)
