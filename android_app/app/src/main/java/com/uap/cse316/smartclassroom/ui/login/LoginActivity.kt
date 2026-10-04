package com.uap.cse316.smartclassroom.ui.login

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.uap.cse316.smartclassroom.databinding.ActivityLoginBinding
import com.uap.cse316.smartclassroom.ui.main.MainActivity

class LoginActivity : AppCompatActivity() {

    private lateinit var binding: ActivityLoginBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityLoginBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.btnLogin.setOnClickListener {
            handleLogin()
        }
    }

    private fun handleLogin() {
        val email = binding.etEmail.text?.toString()?.trim().orEmpty()
        val password = binding.etPassword.text?.toString()?.trim().orEmpty()

        binding.tvError.visibility = View.GONE

        if (email.isEmpty()) {
            binding.tilEmail.error = "Please enter faculty / admin email"
            return
        } else {
            binding.tilEmail.error = null
        }

        if (password.isEmpty()) {
            binding.tilPassword.error = "Please enter password"
            return
        } else {
            binding.tilPassword.error = null
        }

        // Validate Authorized Access (Teacher / Admin as per Project Proposal Section 4)
        val isTeacher = email.equals("teacher@uap.edu", ignoreCase = true) || email.contains("teacher", ignoreCase = true)
        val isAdmin = email.equals("admin@uap.edu", ignoreCase = true) || email.contains("admin", ignoreCase = true)

        if ((isTeacher || isAdmin) && password.length >= 6) {
            val roleName = if (isAdmin) "Administrator" else "Teacher (Sayma Ma'am)"
            Toast.makeText(this, "Welcome, $roleName!", Toast.LENGTH_SHORT).show()

            val intent = Intent(this, MainActivity::class.java).apply {
                putExtra("USER_EMAIL", email)
                putExtra("USER_ROLE", roleName)
            }
            startActivity(intent)
            finish()
        } else {
            binding.tvError.visibility = View.VISIBLE
            binding.tvError.text = "Access Denied: Only authorized faculty/admin accounts can log in."
        }
    }
}
