data = readmatrix('registros/datos_18.02_sincarga_p1.csv');
tiempo = data((2:end),1);
temperatura = data((2:end),4);

datos = iddata(temperatura, tiempo, tiempo(78) - tiempo(1));
sys = tfest(datos,1,1);
%sys = tfest(iddata(temperatura, tiempo_min, mean(diff(tiempo_min))), 2, 1);

% Mostrar la función de transferencia identificada
disp(sys);

% Graficar la respuesta del modelo ajustado
figure;
hold on;
plot(tiempo, temperatura, 'g', 'LineWidth', 1.2);
[y_model, t_model] = step(sys, max(tiempo));
plot(t_model, y_model, 'm--', 'LineWidth', 1.2);
legend('Datos Reales', 'Modelo Ajustado');
xlabel('Tiempo (seg)');
ylabel('Temperatura (°C)');
title('Ajuste del Modelo PID');
grid on;
hold off;

figure;
step(sys);
title('Respuesta al Escalón del Modelo Ajustado');