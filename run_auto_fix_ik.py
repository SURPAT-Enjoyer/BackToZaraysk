import unreal
import sys
import os

# Добавляем путь к Python скриптам
sys.path.append(os.path.join(os.getcwd(), 'Content', 'Python'))

# Импортируем и запускаем автоматическое исправление
try:
    exec(open('Content/Python/AutoFixABP_Player_IK.py').read())
except Exception as e:
    print(f"Ошибка выполнения: {e}")
    # Пытаемся альтернативный способ
    try:
        import AutoFixABP_Player_IK
        AutoFixABP_Player_IK.auto_fix_abp_player_ik()
    except Exception as e2:
        print(f"Альтернативный способ тоже не сработал: {e2}")
