data = readmatrix('registros/datos_18.02_sincarga.csv');
tiempo = data(:,1);
temperatura = data(:,3);

tiempo_min = tiempo/60; 


%Ajustar a un modelo de primer orden (planta)
% herramienta de identificación
%datos = iddata(temperatura, tiempo,tiempo(215) - tiempo(1));
sys = tfest(iddata(temperatura, tiempo_min, mean(diff(tiempo_min))), 2, 1);

% Mostrar la función de transferencia identificada
disp(sys);

% Graficar la respuesta del modelo ajustado
figure;
hold on;
plot(tiempo_min, temperatura, 'g', 'LineWidth', 1.2);
[y_model, t_model] = step(sys, max(tiempo_min));
plot(t_model, y_model, 'm--', 'LineWidth', 1.2);
legend('Datos Reales', 'Modelo Ajustado');
xlabel('Tiempo (min)');
ylabel('Temperatura (°C)');
title('Ajuste del Modelo PID');
grid on;
hold off;

figure;
step(sys);
title('Respuesta al Escalón del Modelo Ajustado');