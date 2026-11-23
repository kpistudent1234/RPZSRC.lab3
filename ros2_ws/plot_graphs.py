import csv
import matplotlib.pyplot as plt

def read_csv(filename):
    data = {}
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for col in reader.fieldnames:
            data[col] = []
        for row in reader:
            for col in reader.fieldnames:
                data[col].append(float(row[col]))
    return data

main_data = read_csv('/ros2_ws/sim_main.csv')
sens_data = read_csv('/ros2_ws/sim_sensitivity.csv')

# Графік 1: Порівняння 3 режимів (Ідеал, Затримка 150 мс, Компенсація)
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

ax1.axhline(0.05, color='black', linestyle='--', linewidth=1.2, label='Цільове положення (0.05 рад)')
ax1.plot(main_data['t'], main_data['x_ideal'], 'g-', linewidth=2, label='Без затримок (Δt = 0 мс)')
ax1.plot(main_data['t'], main_data['x_delayed'], 'r-', linewidth=2, label='Із затримкою (Δt = 150 мс, автоколивання)')
ax1.plot(main_data['t'], main_data['x_comp'], 'b--', linewidth=2.2, label='З лінійним предиктором (компенсація)')
ax1.set_ylabel('Положення x (рад)', fontsize=11)
ax1.set_title('Перехідні процеси П-регулятора суглоба (Kp=5.0, T=50 мс, amax=0.5 рад/с²)', fontsize=12, fontweight='bold')
ax1.grid(True, linestyle=':', alpha=0.7)
ax1.legend(loc='lower right')

ax2.axhline(0.0, color='black', linestyle=':', linewidth=1)
ax2.plot(main_data['t'], main_data['v_ideal'], 'g-', linewidth=2, label='Швидкість (без затримок)')
ax2.plot(main_data['t'], main_data['v_delayed'], 'r-', linewidth=2, label='Швидкість (із затримкою 150 мс)')
ax2.plot(main_data['t'], main_data['v_comp'], 'b--', linewidth=2.2, label='Швидкість (з компенсацією)')
ax2.set_xlabel('Час t (с)', fontsize=11)
ax2.set_ylabel('Швидкість v (рад/с)', fontsize=11)
ax2.grid(True, linestyle=':', alpha=0.7)
ax2.legend(loc='upper right')

plt.tight_layout()
plt.savefig('/ros2_ws/plot_1_compensation.png', dpi=200)
plt.close()

# Графік 2: Вплив параметрів Kp та величини затримки Δt_loop
plt.figure(figsize=(10, 5))
plt.axhline(0.05, color='black', linestyle='--', linewidth=1.2, label='Цільове положення (0.05 рад)')
plt.plot(sens_data['t'], sens_data['x_kp_low'], 'g-', linewidth=2, label='Kp = 2.5, Δt = 150 мс (затухання)')
plt.plot(sens_data['t'], sens_data['x_base_delay'], 'b-', linewidth=2, label='Kp = 5.0, Δt = 150 мс (граничний цикл)')
plt.plot(sens_data['t'], sens_data['x_kp_high'], 'orange', linewidth=2, label='Kp = 8.0, Δt = 150 мс (зростання амплітуди)')
plt.plot(sens_data['t'], sens_data['x_dt_250ms'], 'r--', linewidth=2, label='Kp = 5.0, Δt = 250 мс (збільшена затримка)')
plt.xlabel('Час t (с)', fontsize=11)
plt.ylabel('Положення x (рад)', fontsize=11)
plt.title('Аналіз впливу коефіцієнта підсилення Kp та сумарної затримки Δt_loop на стійкість', fontsize=12, fontweight='bold')
plt.grid(True, linestyle=':', alpha=0.7)
plt.legend(loc='upper right')

plt.tight_layout()
plt.savefig('/ros2_ws/plot_2_sensitivity.png', dpi=200)
plt.close()

print("Графіки успішно збережено: plot_1_compensation.png та plot_2_sensitivity.png")
